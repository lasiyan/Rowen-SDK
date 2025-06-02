#include <rowen/core/exception.hpp>
#include <rowen/network/listener_dgram.hpp>

namespace rs {
namespace network {

const dgram_listener::argument dgram_listener::default_arguments_ = {};

bool dgram_listener::running(int port, const argument& args)
{
  try
  {
    std::lock_guard<std::mutex> locker(listener_lock_);

    // create socket
    if (socket_.open(SOCK_DGRAM, args.socket_protocol) == false)
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

    // set socket option : recv default flags
    socket_.setReceiveFlags(args.listener_recv_flags);

    // set socket option : recv timeout
    if (socket_.setReceiveTimeout(args.listener_recv_timeout) == false)
      throw rs::exception("set recv_timeout : " + socket_.error());

    // else
    // {
    //   // Minimum timeout (for sending)
    //   if (socket_.setReceiveTimeout(1) == false)
    //     throw rs::exception("set default recv_timeout : " + socket_.error());
    // }

    // set socket option : send default flags
    socket_.setSendFlags(args.listener_send_flags);

    // set socket option : send buffer size
    socket_.setSendBufferSize(args.listener_send_buffer_size);

    // set socket option : send timeout
    if (socket_.setSendTimeout(args.listener_send_timeout) == false)
      throw rs::exception("set send_timeout : " + socket_.error());

    // bind socket
    if (socket_.bind(port) == false)
      throw rs::exception("bind socket : " + socket_.error());

    // running listener
    receiver_ = std::async(std::launch::async, &dgram_listener::onReceiveMessage, this, args);
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

ssize_t dgram_listener::send(const uint8_t*            data,
                             const size_t              size,
                             const struct sockaddr_in& addr,
                             const socklen_t           addr_len,
                             int                       send_flags)
{
  ssize_t res = 0;

  {
    std::lock_guard<std::mutex> locker(listener_lock_);
    res = socket_.send(data, size, send_flags, (const struct sockaddr*)&addr, addr_len);
  }

  if (res <= 0)
    error_ = socket_.error();

  return res;
}

ssize_t dgram_listener::send(const rs::Packet&         packet,
                             const struct sockaddr_in& addr,
                             const socklen_t           addr_len,
                             int                       send_flags)
{
  packet.updateTimestamp();

  return send(packet.data(), packet.size(), addr, addr_len, send_flags);
}

std::string dgram_listener::client_address(const struct sockaddr* addr, socklen_t addr_len)
{
  if (addr == nullptr)
    return "";

  char buffer[INET6_ADDRSTRLEN] = {};

  if (addr->sa_family == AF_INET)
  {
    auto addr_in = reinterpret_cast<const struct sockaddr_in*>(addr);
    inet_ntop(AF_INET, &addr_in->sin_addr, buffer, INET_ADDRSTRLEN);
  }
  else if (addr->sa_family == AF_INET6)
  {
    auto addr_in6 = reinterpret_cast<const struct sockaddr_in6*>(addr);
    inet_ntop(AF_INET6, &addr_in6->sin6_addr, buffer, INET6_ADDRSTRLEN);
  }

  return buffer;
}

void dgram_listener::onReceiveMessage(const argument& args)
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

    // wait until either socket has data ready to be recv()
    timeval timeout  = float_to_timeval(timeout_sec);
    auto    activity = ::select(socket_.id() + 1, &read_fds, NULL, NULL, &timeout);
    if (activity < 0)
    {
      error_ = "select error : " + std::string(::strerror(errno));
      continue;
    }

    if (activity == 0)  // timeout
      continue;

    if (FD_ISSET(socket_.id(), &read_fds))
    {
      struct sockaddr_in client_addr     = {};
      socklen_t          client_addr_len = sizeof(client_addr);

      auto buffer = std::make_unique<uint8_t[]>(args.listener_recv_max_size);
      memset(buffer.get(), 0, args.listener_recv_max_size);

      ssize_t recv_len = -1;

      {
        recv_len = socket_.recv(buffer.get(),
                                args.listener_recv_max_size,
                                args.listener_recv_flags,
                                (struct sockaddr*)&client_addr,
                                &client_addr_len);
      }

      if (recv_len <= 0)
      {
        continue;
      }
      else if (callback_received_)
      {
        callback_received_(buffer.get(),
                           recv_len,
                           reinterpret_cast<const sockaddr*>(&client_addr),
                           client_addr_len);
      }
    }
  }
}

void dgram_listener::attachReceivedCallback(const OnReceivedCallback& callback)
{
  std::lock_guard<std::mutex> locker(listener_lock_);

  callback_received_ = callback;
}

}  // namespace network
}  // namespace rs
