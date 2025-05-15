#pragma once

#include <netinet/in.h>  // AF_INET
#include <sys/socket.h>
#include <sys/un.h>  // AF_UNIX

#include <mutex>
#include <string>

namespace rs {
namespace transport {

static constexpr int INVALID_SOCKET           = -1;
static constexpr int DEFAULT_SEND_BUFFER_SIZE = 1024;
static constexpr int DEFAULT_SEND_FLAG        = 0;
static constexpr int DEFAULT_RECV_BUFFER_SIZE = 8192;
static constexpr int DEFAULT_RECV_FLAG        = 0;

namespace internal {

template <typename AddressType>
class SocketBase
{
 public:
  virtual ~SocketBase();

  /**
   * @brief open socket
   * @param socket_domain : AF_UNIX, AF_INET, ...
   * @param socket_type : SOCK_STREAM, SOCK_DGRAM, ...
   * @param protocol : IPPROTO_TCP, IPPROTO_UDP, ...
   * @return true if success, otherwise false
   */
  virtual bool open(int socket_domain, int socket_type, int protocol = 0);

  /**
   * @brief close socket
   */
  virtual void close();

  /**
   * @brief shutdown socket
   * @param how : SHUT_RD, SHUT_WR, SHUT_RDWR
   * @return true if success, otherwise false
   */
  virtual bool shutdown(int how);

  /**
   * @brief check socket validation
   * @return true if valid, otherwise false
   */
  virtual bool valid() const;

  /**
   * @brief bind socket
   * @param port_or_domain_path : port number (for inet) or domain path (for unix)
   * @return true if success, otherwise false
   */
  virtual bool bind(const int port);
  virtual bool bind(const std::string& domain_path);

  /**
   * @brief listen socket
   * @param backlog : listen backlog
   * @return true if success, otherwise false
   */
  virtual bool listen(int backlog = 5);

  /**
   * @brief accept socket
   * @param client : client socket
   * @return true if success, otherwise false
   */
  virtual SocketBase* accept();

  /**
   * @brief check socket properties
   */
  bool isUnix() const { return socket_domain_ == AF_UNIX; }
  bool isInet() const { return socket_domain_ == AF_INET; }
  bool isInet6() const { return socket_domain_ == AF_INET6; }

  /**
   * @brief getter methods
   */
  int                id() const { return socket_fd_; }
  int                type() const { return socket_type_; }
  int                domain() const { return socket_domain_; }
  const char*        error() const { return error_.c_str(); }
  AddressType*       sockaddr() { return &sockaddr_; }
  const AddressType* sockaddr() const { return &sockaddr_; }
  socklen_t          sockaddrSize() const { return sizeof(sockaddr_); }

 protected:
  void setError(const char* function_name, const std::string& user_message = "") const;

 protected:
  int         socket_fd_     = -1;  // socket file descriptor
  int         socket_type_   = -1;  // socket type
  int         socket_domain_ = -1;  // socket domain (AF_UNIX, AF_INET, ...)
  AddressType sockaddr_;            // socket address

  mutable std::string          error_;
  mutable std::recursive_mutex mutex_;
};

};  // namespace internal
};  // namespace transport
};  // namespace rs
