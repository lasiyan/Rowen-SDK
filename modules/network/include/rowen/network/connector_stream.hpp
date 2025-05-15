#pragma once

#include <atomic>
#include <mutex>

#include "rowen/transport/packet_typedef.hpp"
#include "template/connector.hpp"

namespace rs {
namespace network {

class stream_connector : public template_connector
{
 public:
  struct argument
  {
    int   socket_protocol = 0;
    float connect_timeout = 1;  // timeout is seconds

    float send_timeout     = 1;  // timeout is seconds
    int   send_flags       = MSG_NOSIGNAL;
    int   send_buffer_size = DEFAULT_SEND_BUFFER_SIZE;
    bool  send_retry       = false;
    float recv_timeout     = 0;  // timeout is seconds
    int   recv_flags       = MSG_NOSIGNAL;
    bool  recv_retry       = false;  // timeout이 설정되어 있을 때 retry 여부
  };

 public:
  /**
   * @brief Initialize connector configuration (Optional. Use connect() instead)
   * @param domain_file : absolute path like "/tmp/domain.sock"
   * @param argument : connector argument
   */
  void initialize(const std::string& server_address,
                  const int          server_port,
                  const argument&    arguments = default_arguments_);

  /**
   * @brief Connect to server
   * @param server_address : server address
   * @param server_port : server port
   * @param argument : connector argument
   */
  bool connect(const std::string& server_address,
               const int          server_port,
               const argument&    arguments = default_arguments_);

  /**
   * @brief Disconnect from server
   */
  void disconnect();

  /**
   * @brief Check connection status
   * @return true if connected, otherwise false
   */
  bool isConnected() const { return connected_; }

  /**
   * @brief Send data to server
   */
  ssize_t send(const uint8_t* data, size_t size, int send_flags = -1, int retry_flag = -1);

  /**
   * @brief Send data to server (with rs packet)
   */
  ssize_t send(const rs::Packet& packet, int send_flags = -1, int retry_flag = -1);

  /**
   * @brief Receive data from server
   */
  ssize_t recv(uint8_t* data, size_t size, int recv_flags = -1);

  /**
   * @brief Send and receive data from server
   * @details recv 값이 전달되지 않으면 내부적으로 로컬 버퍼를 사용하여 recv를 수행합니다.
   */
  ssize_t sendToRecv(const uint8_t* send_data, const size_t send_size,
                     uint8_t* recv_data = nullptr, const size_t recv_size = 0,
                     int send_flags = -1, int recv_flags = -1);

 private:
  ssize_t try_send(const uint8_t* data, size_t size, int send_flags = -1);

 private:
  static const argument default_arguments_;

 private:
  std::atomic_bool connected_        = false;
  std::mutex       connector_locker_ = {};

  // caching for reconnection
  argument last_arguments_ = {};
};

};  // namespace network
};  // namespace rs
