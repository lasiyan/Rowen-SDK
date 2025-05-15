#pragma once

#include "rowen/transport/packet_typedef.hpp"
#include "template/listener.hpp"

namespace rs {
namespace network {

class dgram_listener : public template_listener
{
 public:
  struct argument
  {
    int  socket_protocol = 0;
    bool reuse_address   = true;
    bool reuse_port      = true;

    float listener_select_timeout   = 1;  // timeout is seconds
    float listener_recv_timeout     = 0;  // timeout is seconds
    int   listener_recv_flags       = MSG_NOSIGNAL;
    int   listener_recv_max_size    = DEFAULT_RECV_BUFFER_SIZE;
    float listener_send_timeout     = 1;  // timeout is seconds
    int   listener_send_flags       = MSG_NOSIGNAL;
    int   listener_send_buffer_size = DEFAULT_SEND_BUFFER_SIZE;
  };

  // callback
  using OnReceivedCallback = std::function<void(const uint8_t*, size_t, const struct sockaddr*, const socklen_t)>;

 public:
  /**
   * @brief running listener
   * @param port : port number
   * @param arguments : listener arguments
   * @return true if success, otherwise false
   */
  bool running(int port, const argument& arguments = default_arguments_);

  /**
   * @brief send listener
   * @param data : data buffer (with. unsigned char*)
   * @param size : data size (byte)
   * @param addr : destination address (a.k.a. Client address)
   * @param addr_len : destination address length (a.k.a. Client address length)
   * @param send_flags : send flag (default : listener argument listener_send_flags)
   */
  ssize_t send(const uint8_t*            data,
               const size_t              size,
               const struct sockaddr_in& addr,
               const socklen_t           addr_len,
               int                       send_flags = -1);

  /**
   * @brief send listener
   * @param packet : rs packet struct
   * @param addr : destination address (a.k.a. Client address)
   * @param addr_len : destination address length (a.k.a. Client address length)
   * @param send_flags : send flag (default : listener argument listener_send_flags)
   */
  ssize_t send(const rs::Packet&         packet,
               const struct sockaddr_in& addr,
               const socklen_t           addr_len,
               int                       send_flags = -1);

  /**
   * @brief get client address
   * @param addr : client address
   * @param addr_len : client address length
   */
  static std::string client_address(const struct sockaddr* addr, socklen_t addr_len);

 public:
  /**
   * @brief attach listener callbacks
   * @param callback : listener callback funtion pointer
   */
  void attachReceivedCallback(const OnReceivedCallback& callback);

 private:
  void onReceiveMessage(const argument& arguments);

 private:
  static const argument default_arguments_;

 private:
  OnReceivedCallback callback_received_ = nullptr;
};

};  // namespace network
};  // namespace rs
