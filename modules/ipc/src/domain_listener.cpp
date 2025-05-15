#include <sys/un.h>

#include <filesystem>

#include "rowen/ipc/domain/listener.hpp"

namespace rs {
namespace ipc {
namespace domain {

const listener::argument listener::default_arguments_ = {};

listener::~listener()
{
  stop();
}

bool listener::running(const std::string& domain_path, const argument& args)
{
  try
  {
    std::lock_guard<std::mutex> locker(listener_lock_);

    // create socket
    if (socket_.open(SOCK_STREAM) == false)
      throw rs::exception("open socket : " + socket_.error());

    // set reuse domain
    if (args.reuse_domain)
      ::unlink(domain_path.c_str());
    else if (std::filesystem::exists(domain_path))
      throw rs::exception("domain session is already exists : " + domain_path);

    // update domain path
    domain_path_ = domain_path;

    // set socket option : recv timeout
    if (socket_.setReceiveTimeout(args.listener_recv_timeout) == false)
      throw rs::exception("set recv_timeout : " + socket_.error());

    // bind socket
    if (socket_.bind(domain_path) == false)
      throw rs::exception("bind socket : " + socket_.error());

    // Listen for incoming connections
    if (socket_.listen(args.backlog) == false)
      throw rs::exception("listen : " + socket_.error());

    // running listener
    receiver_ = std::async(std::launch::async, &listener::onReceiveMessage, this, args);
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

void listener::stop()
{
  std::lock_guard<std::mutex> locker(listener_lock_);

  receiver_stop_ = true;
  if (receiver_.valid())
    receiver_.wait();

  socket_.close();

  ::unlink(domain_path_.c_str());
}

ssize_t listener::send(const Client* client, const uint8_t* data, int size, int send_flag)
{
  if (client == nullptr)
  {
    error_ = "invalid client";
    return false;
  }

  ssize_t res = 0;

  {
    std::lock_guard<std::mutex> locker(connected_clients_mutex_);
    res = client->send(data, size, send_flag);
  }

  // error handle
  if (res <= 0)
    error_ = client->error();

#if 0  // recv에서 처리
  if (res == 0)  // client disconnected
  {
    // client disconnected
    if (callback_disconnected_)
      callback_disconnected_(client);

    // remove client
    {
      std::lock_guard<std::mutex> locker(connected_clients_mutex_);
      connected_clients_.erase(*client);
    }
  }
#endif

  return res;
}

ssize_t listener::send(const Client* client, const rs::Packet& packet, int send_flag)
{
  return send(client, packet.data(), packet.size(), send_flag);
}

void listener::onReceiveMessage(const argument& args)
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

    for (const auto& client_socket : connected_clients_)
    {
      FD_SET(client_socket.id(), &read_fds);
      if (client_socket.id() > fd_max)
        fd_max = client_socket.id();
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
        if (connected_clients_.find(accepted_socket) != connected_clients_.end())
        {
          error_ = "already connected client : " + std::to_string(accepted_socket.id());
          continue;
        }

        new_client = &(*connected_clients_.insert(accepted_socket).first);

        if (new_client)
        {
          new_client->setReceiveFlags(args.client_recv_flags);
          new_client->setReceiveTimeout(args.client_recv_timeout);
          new_client->setSendFlags(args.client_send_flags);
          new_client->setSendTimeout(args.client_send_timeout);
          new_client->setSendBufferSize(args.client_buffer_size);
        }
      }

      // call connected callback
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
      auto& client_socket = *iter;

      if (FD_ISSET(client_socket.id(), &read_fds) == false)
      {
        ++iter;
        continue;
      }

      // receive data
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

          oss << "[TRACE RS PACKET END]"
              << "\033[0m" << std::endl;

          printf("%s", oss.str().c_str());
        }
      }

      if (recv_size == 0)
      {
        // client disconnected
        if (callback_disconnected_)
          callback_disconnected_(&(*iter));

        // remove client
        {
          std::lock_guard<std::mutex> locker(connected_clients_mutex_);
          const_cast<Socket&>(*iter).close();
          iter = connected_clients_.erase(iter);
        }

        // remove packet collector
        if (args.using_rs_packet)
        {
          auto collector_iter = rs_packet_receiver_.find(&(*iter));
          if (collector_iter != rs_packet_receiver_.end())
            rs_packet_receiver_.erase(collector_iter);
        }

        continue;
      }
      else if (recv_size < 0)
      {
        error_ = "recv error : " + std::string(::strerror(errno));
      }
      else
      {
        if (args.using_rs_packet == false)
        {
          if (callback_received_)
            callback_received_(&client_socket, buffer.get(), recv_size);
        }
        else
        {
          auto& receiver = rs_packet_receiver_.find(&(*iter))->second;

          // regist callback for grab packet when received completed
          receiver->attachCallback([&](const uint8_t* data, const ssize_t size) {
            if (callback_received_)
              callback_received_(&client_socket, data, size);
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

void listener::attachConnectedCallback(const OnConnectedCallback& callback)
{
  std::lock_guard<std::mutex> locker(listener_lock_);

  callback_connected_ = callback;
}

void listener::attachDisconnectedCallback(const OnDisconnectedCallback& callback)
{
  std::lock_guard<std::mutex> locker(listener_lock_);

  callback_disconnected_ = callback;
}

void listener::attachReceivedCallback(const OnReceivedCallback& callback)
{
  std::lock_guard<std::mutex> locker(listener_lock_);

  callback_received_ = callback;
}

const listener::client_set& listener::clients() const
{
  std::lock_guard<std::mutex> locker(connected_clients_mutex_);

  return connected_clients_;
}

const listener::Client* listener::client(const int client_id) const
{
  std::lock_guard<std::mutex> locker(connected_clients_mutex_);

  for (const auto& client : connected_clients_)
  {
    if (client.id() == client_id)
      return &client;
  }

  return nullptr;
}

void listener::disconnect(const Client* client, const int how)
{
  if (client == nullptr)
    return;

  client->shutdown(how);
}

}  // namespace domain
}  // namespace ipc
}  // namespace rs
