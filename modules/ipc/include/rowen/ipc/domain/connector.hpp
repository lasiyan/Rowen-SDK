#pragma once

#include <atomic>
#include <mutex>

#include "template/socket.hpp"

namespace rs {
namespace ipc {
namespace domain {

class connector
{
 public:
  struct argument
  {
    float connect_timeout  = 1;  // timeout is seconds
    float send_timeout     = 1;  // timeout is seconds
    int   send_flags       = MSG_NOSIGNAL;
    int   send_buffer_size = DEFAULT_SEND_BUFFER_SIZE;
    bool  send_retry       = false;
    float recv_timeout     = 0;  // timeout is seconds
    int   recv_flags       = MSG_NOSIGNAL;
    bool  recv_retry       = false;  // timeout이 설정되어 있을 때 retry 여부
  };

 public:
  virtual ~connector();

  /**
   * @brief Initialize connector configuration (Optional. Use connect() instead)
   * @param domain_file : absolute path like "/tmp/domain.sock"
   * @param argument : connector argument
   */
  void initialize(const std::string& domain_file,
                  const argument&    arguments = default_arguments_);

  /**
   * @brief Connect to UDS server
   * @param domain_file : absolute path like "/tmp/domain.sock"
   * @param argument : connector argument
   */
  bool connect(const std::string& domain_file,
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
   * @param data : data buffer (with. unsigned char*)
   * @param size : data size (byte)
   * @param send_flags : send flag (default : connector argument send_flags)
   */
  ssize_t send(const uint8_t* data, size_t size, int send_flags = -1, int retry_flag = -1);

  /**
   * @brief Send data to server
   * @param packet : rs packet struct
   * @param send_flags : send flag (default : connector argument send_flags)
   */
  ssize_t send(const rs::Packet& packet, int send_flags = -1, int retry_flag = -1);

  /**
   * @brief Receive data from server
   * @param data : data buffer (with. unsigned char*)
   * @param size : data size (byte)
   * @param recv_flags : receive flag (default : connector argument recv_flags)
   */
  ssize_t recv(uint8_t* data, size_t size, int recv_flags = -1);

  /**
   * @brief Send and receive data from server
   * @details recv 값이 전달되지 않으면 내부적으로 로컬 버퍼를 사용하여 recv를 수행합니다.
   */
  ssize_t sendToRecv(const uint8_t* send_data, const size_t send_size,
                     uint8_t* recv_data = nullptr, const size_t recv_size = 0,
                     int send_flags = -1, int recv_flags = -1);

  /**
   * @brief get socket
   * @return Socket class instance
   */
  class Socket& socket() { return socket_; }

  /**
   * @brief get last error message
   * @return error message
   */
  const char* error() const { return error_.empty() ? nullptr : error_.c_str(); }

 private:
  ssize_t try_send(const uint8_t* data, size_t size, int send_flags = -1);

 private:
  static const argument default_arguments_;

 private:
  class Socket     socket_           = {};
  std::string      error_            = "";
  std::atomic_bool connected_        = false;
  std::mutex       connector_locker_ = {};

  // caching for reconnection
  argument    last_arguments_ = {};
  std::string last_domain_    = "";
};

};  // namespace domain
};  // namespace ipc
};  // namespace rs
