#pragma once

#include <mutex>
#include <rowen/core/transport/packet_receiver.hpp>
#include <rowen/network/detail/connector.hpp>

namespace rs {
namespace network {

class dgram_connector : public template_connector
{
 public:
  struct argument
  {
    int socket_protocol = 0;

    float send_timeout     = 1;  // timeout is seconds
    int   send_flags       = MSG_NOSIGNAL;
    int   send_buffer_size = DEFAULT_SEND_BUFFER_SIZE;
    float recv_timeout     = 0;  // timeout is seconds
    int   recv_flags       = MSG_NOSIGNAL;
  };

 public:
  /**
   * @brief Connect to server
   * @param server_address : server address
   * @param server_port : server port
   * @param argument : connector argument
   */
  bool initialize(const std::string& server_address,
                  const int          server_port,
                  const argument&    arguments = default_arguments_);

  /**
   * @brief Send data to server (from preset)
   */
  ssize_t sendto(const uint8_t* data, size_t size, int send_flags = -1);

  /**
   * @brief Send data to server
   */
  ssize_t sendto(const uint8_t* data, size_t size, const struct sockaddr* addr, const socklen_t addrlen, int send_flags = -1);

  /**
   * @brief Send data to server (from preset with rs packet)
   */
  ssize_t sendto(const rs::Packet& packet, int send_flags = -1);

  /**
   * @brief Send data to server (with rs packet)
   */
  ssize_t sendto(const rs::Packet& packet, const struct sockaddr* addr, const socklen_t addrlen, int send_flags = -1);

  /**
   * @brief Receive data from server (from preset)
   */
  ssize_t recvfrom(uint8_t* data, size_t size, int recv_flags = -1);

  /**
   * @brief Receive data from server
   */
  ssize_t recvfrom(uint8_t* data, size_t size, struct sockaddr* addr, socklen_t* addrlen, int recv_flags = -1);

 private:
  static const argument default_arguments_;

 private:
  sockaddr_in server_addr_      = {};
  socklen_t   server_addr_len_  = sizeof(server_addr_);
  std::mutex  connector_locker_ = {};
};

};  // namespace network
};  // namespace rs
