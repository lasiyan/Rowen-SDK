#include "rowen/network/listener_stream.hpp"

#include <iomanip>
#include <sstream>

#include "rowen/core/exception.hpp"

namespace rs {
namespace network {

const stream_listener::argument stream_listener::default_arguments_ = {};

bool stream_listener::running(int port, const argument& args)
{
  try
  {
    std::lock_guard<std::mutex> locker(listener_lock_);

    // create socket
    if (socket_.open(SOCK_STREAM, args.socket_protocol) == false)
      throw rs::exception("open socket : " + socket_.error());

    // set socket option : reuse address
    {
      int reuse_address = args.reuse_address ? 1 : 0;
      if (socket_.setOption(SOL_SOCKET, SO_REUSEADDR, &reuse_address, sizeof(reuse_address)) == false)
        throw rs::exception("set reuse_addr : " + socket_.error());
    }

    // set socket option : reuse port
    {
      int reuse_port = args.reuse_port ? 1 : 0;
      if (socket_.setOption(SOL_SOCKET, SO_REUSEPORT, &reuse_port, sizeof(reuse_port)) == false)
        throw rs::exception("set reuse_port : " + socket_.error());
    }

    // set socket option : recv timeout
    if (socket_.setReceiveTimeout(args.listener_recv_timeout) == false)
      throw rs::exception("set recv_timeout : " + socket_.error());

    // bind socket
    if (socket_.bind(port) == false)
      throw rs::exception("bind socket : " + socket_.error());

    // Listen for incoming connections
    if (socket_.listen(args.backlog) == false)
      throw rs::exception("listen : " + socket_.error());

    // running listener
    receiver_ = std::async(std::launch::async, &stream_listener::onReceiveMessage, this, args);
  }
  catch (const rs::exception& e)
  {
    // release socket
    socket_.close();

    // set last error
    error_ = e.what();
    return false;
  }

  return true;
}

void stream_listener::onReceiveMessage(const argument& args)
{
  receiver_stop_ = false;

  // timeout (restrict infinite timeout)
  float timeout_sec = args.listener_select_timeout;
  if (timeout_sec < 0.01)
    timeout_sec = 0.5;

  while (receiver_stop_ == false)
  {
    // init fd_set
    fd_set read_fds;
    FD_ZERO(&read_fds);

    // add our descriptors to the set
    FD_SET(socket_.id(), &read_fds);
    int fd_max = socket_.id();

    for (const auto& [_, client] : connected_clients_)
    {
      FD_SET(client.client_socket.id(), &read_fds);
      if (client.client_socket.id() > fd_max)
        fd_max = client.client_socket.id();
    }

    // wait until either socket has data ready to be recv()
    timeval timeout  = float_to_timeval(timeout_sec);
    int     activity = ::select(fd_max + 1, &read_fds, NULL, NULL, &timeout);
    if (activity < 0)
    {
      error_ = "select error : " + std::string(::strerror(errno));
      continue;
    }

    if (activity == 0)  // timeout
      continue;

    // If something happened on the master socket, then its an incoming connection
    if (FD_ISSET(socket_.id(), &read_fds))
    {
      auto accepted_socket = socket_.accept();

      if (accepted_socket.valid() == false)
      {
        error_ = "accept error : " + socket_.error();
        continue;
      }

      // add the new socket to the connected_clients_
      const Client* new_client = nullptr;

      {
        std::lock_guard<std::mutex> locker(connected_clients_mutex_);

        // add new client
        if (connected_clients_.find(accepted_socket.id()) != connected_clients_.end())
        {
          error_ = "already connected client : " + std::to_string(accepted_socket.id());
          continue;
        }

        Client client;
        client.client_socket = accepted_socket;
        snprintf(client.client_ipaddr, sizeof(client.client_ipaddr), "%s", inet_ntoa(accepted_socket.address().sin_addr));

        new_client = &connected_clients_.insert({ client.client_socket.id(), client }).first->second;

        if (new_client)
        {
          new_client->client_socket.setReceiveFlags(args.client_recv_flags);
          new_client->client_socket.setReceiveTimeout(args.client_recv_timeout);
          new_client->client_socket.setSendFlags(args.client_send_flags);
          new_client->client_socket.setSendTimeout(args.client_send_timeout);
          new_client->client_socket.setSendBufferSize(args.client_buffer_size);
        }
      }

      // callback
      if (callback_connected_ && new_client)
        callback_connected_(new_client);

      // create packet collector
      if (args.using_rs_packet)
      {
        if (rs_packet_receiver_.find(new_client) == rs_packet_receiver_.end())
          rs_packet_receiver_[new_client] = std::make_unique<PacketReceiver>();
      }
    }

    auto iter = connected_clients_.begin();
    while (iter != connected_clients_.end())
    {
      auto& client_socket = iter->second.client_socket;

      if (FD_ISSET(client_socket.id(), &read_fds) == false)
      {
        ++iter;
        continue;
      }

      // Receive data & Check client disconnection
      auto    buffer    = std::make_unique<uint8_t[]>(args.listener_buffer_size);
      ssize_t recv_size = -1;

      {
        std::lock_guard<std::mutex> locker(connected_clients_mutex_);

        recv_size = client_socket.recv(buffer.get(),
                                       args.listener_buffer_size,
                                       args.client_recv_flags);

        if (args.using_rs_packet && args.trace_rs_packet && recv_size > 0)
        {
          std::ostringstream oss;

          oss << "\033[2;36m"
              << "[TRACE RS PACKET] received size : " << recv_size << std::endl;

          for (int i = 0; i < recv_size; ++i)
            oss << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << (unsigned int)buffer[i] << " ";
          oss << std::endl;

          oss << "[TRACE RS PACKET] end of packet"
              << "\033[0m" << std::endl;

          printf("%s", oss.str().c_str());
        }
      }

      if (recv_size == 0)
      {
        // client disconnected
        if (callback_disconnected_)
          callback_disconnected_(&(iter->second));

        // remove client
        {
          std::lock_guard<std::mutex> locker(connected_clients_mutex_);
          iter->second.client_socket.close();
          iter = connected_clients_.erase(iter);
        }

        // remove packet collector
        if (args.using_rs_packet)
        {
          auto packet_iter = rs_packet_receiver_.find(&(iter->second));
          if (packet_iter != rs_packet_receiver_.end())
            rs_packet_receiver_.erase(packet_iter);
        }

        continue;
      }
      else if (recv_size < 0)
      {
        error_ = "recv error : " + std::string(::strerror(errno));
      }
      else
      {
        // callback
        if (args.using_rs_packet == false)
        {
          if (callback_received_)
            callback_received_(&(iter->second), buffer.get(), recv_size);
        }
        else
        {
          auto& receiver = rs_packet_receiver_.find(&(iter->second))->second;

          // regist callback for grab packet when received completed
          receiver->attachCallback([&](const uint8_t* data, const ssize_t size) {
            if (callback_received_)
              callback_received_(&(iter->second), data, size);
          });

          // store received data to temporary buffer
          receiver->store(buffer.get(), recv_size);

          // after store, progress received data and relese callback
          receiver->resetCallback();
        }
      }

      ++iter;
    }
  }
}

ssize_t stream_listener::send(const Client* client, const uint8_t* data, int size, int send_flag)
{
  if (client == nullptr)
  {
    error_ = "invalid client";
    return false;
  }

  ssize_t res = 0;

  {
    std::lock_guard<std::mutex> locker(connected_clients_mutex_);
    res = client->client_socket.send(data, size, send_flag);
  }

  // error handle
  if (res <= 0)
    error_ = client->client_socket.error();

#if 0  // recv에서 처리
  if (res == 0)  // client disconnected
  {
    // client disconnected
    if (callback_disconnected_)
      callback_disconnected_(client);

    // remove client
    {
      std::lock_guard<std::mutex> locker(connected_clients_mutex_);
      connected_clients_.erase(client->client_socket.id());
    }
  }
#endif

  return res;
}

ssize_t stream_listener::send(const Client* client, const rs::Packet& packet, int send_flag)
{
  packet.updateTimestamp();

  return send(client, packet.data(), packet.size(), send_flag);
}

const std::unordered_map<int, stream_listener::Client>& stream_listener::clients() const
{
  std::lock_guard<std::mutex> locker(connected_clients_mutex_);

  return connected_clients_;
}

const stream_listener::Client* stream_listener::client(int client_socket) const
{
  std::lock_guard<std::mutex> locker(connected_clients_mutex_);

  auto iter = connected_clients_.find(client_socket);
  if (iter == connected_clients_.end())
    return nullptr;

  return &(iter->second);
}

void stream_listener::disconnect(const Client* client, const int how)
{
  if (client == nullptr)
    return;

  client->client_socket.shutdown(how);
}

void stream_listener::attachConnectedCallback(const OnConnectedCallback& callback)
{
  std::lock_guard<std::mutex> locker(listener_lock_);

  callback_connected_ = callback;
}

void stream_listener::attachDisconnectedCallback(const OnDisconnectedCallback& callback)
{
  std::lock_guard<std::mutex> locker(listener_lock_);

  callback_disconnected_ = callback;
}

void stream_listener::attachReceivedCallback(const OnReceivedCallback& callback)
{
  std::lock_guard<std::mutex> locker(listener_lock_);

  callback_received_ = callback;
}

}  // namespace network
}  // namespace rs
