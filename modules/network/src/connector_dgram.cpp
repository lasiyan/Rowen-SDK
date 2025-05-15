
#include "rowen/network/connector_dgram.hpp"

#include <fcntl.h>

#include "rowen/core/exception.hpp"

namespace rs {
namespace network {

const dgram_connector::argument dgram_connector::default_arguments_ = {};

bool dgram_connector::initialize(const std::string& server_address,
                                 const int          server_port,
                                 const argument&    args)
{
  server_ipv4_ = server_address;
  server_port_ = server_port;

  server_addr_.sin_family      = AF_INET;
  server_addr_.sin_addr.s_addr = inet_addr(server_ipv4_.c_str());
  server_addr_.sin_port        = htons(server_port_);

  try
  {
    std::lock_guard<std::mutex> locker(connector_locker_);

    // create socket
    if (socket_.open(SOCK_DGRAM, args.socket_protocol) == false)
      throw rs::exception("open socket : " + socket_.error());

    // set socket option : recv default flags
    socket_.setReceiveFlags(args.recv_flags);

    // set socket option : recv timeout
    if (socket_.setReceiveTimeout(args.recv_timeout) == false)
      throw rs::exception("set recv_timeout : " + socket_.error());

    // set socket option : send default flags
    socket_.setSendFlags(args.send_flags);

    // set socket option : send buffer size
    socket_.setSendBufferSize(args.send_buffer_size);

    // set socket option : send timeout
    if (socket_.setSendTimeout(args.send_timeout) == false)
      throw rs::exception("set send_timeout" + socket_.error());
  }
  catch (const rs::exception& e)
  {
    error_ = e.what();
    return false;
  }

  return true;
}

ssize_t dgram_connector::sendto(const uint8_t* data, size_t size, int send_flags)
{
  server_addr_len_ = sizeof(server_addr_);

  return sendto(data, size, (struct sockaddr*)&server_addr_, server_addr_len_, send_flags);
}

ssize_t dgram_connector::sendto(const uint8_t* data, size_t size, const struct sockaddr* addr, const socklen_t addrlen, int send_flags)
{
  ssize_t res = 0;

  {
    std::lock_guard<std::mutex> locker(connector_locker_);
    res = socket_.send(data, size, send_flags, addr, addrlen);
  }

  if (res <= 0)
    error_ = socket_.error();

  return res;
}

ssize_t dgram_connector::sendto(const rs::Packet& packet, int send_flags)
{
  packet.updateTimestamp();

  return sendto(packet.data(), packet.size(), send_flags);
}

ssize_t dgram_connector::sendto(const rs::Packet& packet, const struct sockaddr* addr, const socklen_t addrlen, int send_flags)
{
  return sendto(packet.data(), packet.size(), addr, addrlen, send_flags);
}

ssize_t dgram_connector::recvfrom(uint8_t* data, size_t size, int recv_flags)
{
  return recvfrom(data, size, (struct sockaddr*)&server_addr_, &server_addr_len_, recv_flags);
}

ssize_t dgram_connector::recvfrom(uint8_t* data, size_t size, struct sockaddr* addr, socklen_t* addrlen, int recv_flags)
{
  ssize_t res = socket_.recv(data, size, recv_flags, addr, addrlen);

  if (res <= 0)
  {
    if (errno == EAGAIN)
    {
      error_ = "receive timeout";
    }
    else
    {
      error_ = socket_.error();
    }
  }

  return res;
}

};  // namespace network
};  // namespace rs
