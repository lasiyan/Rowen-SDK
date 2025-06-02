#pragma once

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <cstdint>
#include <rowen/core/exception.hpp>
#include <rowen/core/transport/packet_typedef.hpp>
#include <string>

namespace rs {
namespace ipc {
namespace domain {

static constexpr int INVALID_SOCKET           = -1;
static constexpr int DEFAULT_SEND_BUFFER_SIZE = 1024;
static constexpr int DEFAULT_SEND_FLAG        = 0;
static constexpr int DEFAULT_RECV_BUFFER_SIZE = 8192;
static constexpr int DEFAULT_RECV_FLAG        = 0;

class Socket
{
 public:
  // 소멸자에서 close()를 호출하려면, 각 listener에 Socket인스턴스를 포인터로 가지고 있어야 한다.
  // ~Socket() { close(); }

  bool open(int socket_type);

  void close();

  bool valid() const;

  bool bind(const std::string& domain_path);

  bool listen(int backlog);

  class Socket accept();

  bool connect(const std::string& domain_path) const;

  bool shutdown(int how) const;

  /*
  * @brief send data
  * @param data : data buffer
  * @param size : data buffer size
  * @param flags : socket flags
  * @param address : receiver address (IF socket is based on SOCK_DGRAM, otherwise nullptr)
  * @param addr_len : receiver address length (IF socket is based on SOCK_DGRAM, otherwise -1)
  * @return sent data size (IF error, check reason using error() or cerror())
    return value '-1' means error state. return value '0' means send is not available by connection (so. need to reconnect)
  */
  ssize_t send(const uint8_t* data, size_t size, int flags = -1,
               const struct sockaddr* address = nullptr, socklen_t addr_len = -1) const;

  /*
  * @brief send data
  * @param packet : data packet
  * @param flags : socket flags
  * @param address : receiver address (IF socket is based on SOCK_DGRAM, otherwise nullptr)
  * @param addr_len : receiver address length (IF socket is based on SOCK_DGRAM, otherwise -1)
  * @return sent data size (IF error, check reason using error() or cerror())
    return value '-1' means error state. return value '0' means send is not available by connection (so. need to reconnect)
  */
  ssize_t send(const rs::Packet& packet, int flags = -1,
               const struct sockaddr* address = nullptr, socklen_t addr_len = -1) const;

  /*
  * @brief receive data
  * @param data : data buffer
  * @param size : data buffer size
  * @param flags : socket flags
  * @param address : sender address (IF socket is based on SOCK_DGRAM, otherwise nullptr)
  * @param addr_len : sender address length (IF socket is based on SOCK_DGRAM, otherwise nullptr)
  * @return received data size (IF error, check reason using error() or cerror())
    return value '-1' means error state. return value '0' means receive is not available by connection (so. need to reconnect)
  */
  ssize_t recv(void* data, size_t size, int flags = -1,
               struct sockaddr* address = nullptr, socklen_t* addr_len = nullptr) const;

  bool setOption(int level, int option, const void* value, socklen_t size) const;

 public:
  // getter
  int         id() const { return handle_; }
  sockaddr_un address() const { return sockaddr_; }
  size_t      address_size() const { return sizeof(sockaddr_); }
  int         type() const { return props_.socket_type; }

  std::string error() const { return error_message_; }
  const char* cerror() const { return error_message_.c_str(); }

  // setter
  bool setSendTimeout(const float sec) const;
  void setSendBufferSize(int size) const;
  void setSendFlags(int flags) const;
  bool setReceiveTimeout(const float sec) const;
  void setReceiveFlags(int flags) const;

 private:
  int         handle_   = INVALID_SOCKET;
  sockaddr_un sockaddr_ = {};

  mutable std::string error_message_ = "";

  // Socket property
  struct property
  {
    // common
    int socket_type = 0;  // SOCK_STREAM, SOCK_DGRAM, ...
    int backlog     = 0;  // listen backlog

    // send
    struct send
    {
      timeval timeout     = { 0, 500 };
      int     buffer_size = DEFAULT_SEND_BUFFER_SIZE;
      int     base_flags  = DEFAULT_SEND_FLAG;
    } mutable send;

    // recv
    struct recv
    {
      timeval timeout    = { 0, 500 };
      int     base_flags = DEFAULT_RECV_FLAG;
    } mutable recv;
  } props_;
};

struct socket_hash
{
  std::size_t operator()(const Socket& s) const
  {
    return std::hash<int>{}(s.id());
  }
};

struct socket_equal
{
  bool operator()(const Socket& lhs, const Socket& rhs) const
  {
    return lhs.id() == rhs.id();
  }
};

/*
----------------------------------------------------------------------------------
  Implementation
----------------------------------------------------------------------------------
*/
inline bool verify_domain(const std::string& domain)
{
  return (domain.empty() == false);
}

inline void assert_domain(const std::string& domain)
{
  if (verify_domain(domain) == false)
    throw rs::exception("invalid domain name: " + std::string(domain));
}

inline struct timeval float_to_timeval(const float sec)
{
  struct timeval timeout;
  timeout.tv_sec  = static_cast<time_t>(sec);
  timeout.tv_usec = static_cast<suseconds_t>((sec - timeout.tv_sec) * 1000000);
  return timeout;
}

inline bool Socket::open(int socket_type)
{
  close();

  props_.socket_type = socket_type;

  handle_ = ::socket(AF_UNIX, socket_type, 0);

  if (handle_ <= 0)
  {
    error_message_ = ::strerror(errno);
    return false;
  }

  return true;
}

inline void Socket::close()
{
  if (this->valid() == false)
    return;

  {
    ::close(handle_);
    handle_ = INVALID_SOCKET;
  }
}

inline bool Socket::valid() const
{
  return handle_ != INVALID_SOCKET;
}

inline bool Socket::bind(const std::string& domain_path)
{
  if (verify_domain(domain_path) == false)
    return false;

  sockaddr_.sun_family = AF_UNIX;
  strncpy(sockaddr_.sun_path, domain_path.c_str(), sizeof(sockaddr_.sun_path) - 1);

  if (::bind(handle_, (struct sockaddr*)&sockaddr_, sizeof(sockaddr_)) == -1)
  {
    error_message_ = ::strerror(errno);
    return false;
  }

  return true;
}

inline bool Socket::listen(int backlog)
{
  if (backlog <= 0)
  {
    error_message_ = "invalid backlog: " + std::to_string(backlog);
    return false;
  }
  else
    props_.backlog = backlog;

  if (::listen(handle_, backlog) == -1)
  {
    error_message_ = ::strerror(errno);
    return false;
  }

  return true;
}

inline Socket Socket::accept()
{
  struct sockaddr_un accepted_addr      = {};
  socklen_t          accepted_addr_size = sizeof(sockaddr_);

  auto accepted_handle = ::accept(handle_, (struct sockaddr*)&accepted_addr, &accepted_addr_size);

  if (accepted_handle < 0)
  {
    error_message_ = ::strerror(errno);
    return Socket();
  }
  else
  {
    class Socket accepted_socket;
    socklen_t    type_len   = sizeof(accepted_socket.props_.socket_type);
    accepted_socket.handle_ = accepted_handle;
    accepted_socket.props_  = props_;

    if (getsockopt(accepted_handle, SOL_SOCKET, SO_TYPE, &accepted_socket.props_.socket_type, &type_len) == -1)
      accepted_socket.props_.socket_type = props_.socket_type;

    memcpy(&accepted_socket.sockaddr_, &accepted_addr, accepted_addr_size);
    return accepted_socket;
  }
}

inline bool Socket::connect(const std::string& domain_path) const
{
  assert_domain(domain_path);

  sockaddr_un address = {};
  address.sun_family  = AF_UNIX;
  strncpy(address.sun_path, domain_path.c_str(), sizeof(address.sun_path) - 1);

  if (::connect(handle_, (struct sockaddr*)&address, sizeof(address)) == -1)
  {
    error_message_ = ::strerror(errno);
    return false;
  }

  return true;
}

inline bool Socket::shutdown(int how) const
{
  if (this->valid() == false)
    return false;

  if (::shutdown(handle_, how) == -1)
  {
    error_message_ = ::strerror(errno);
    return false;
  }

  return true;
}

inline ssize_t Socket::send(const uint8_t* data, size_t size, int flags, const struct sockaddr* address, socklen_t addr_len) const
{
  constexpr int MAX_RETRY = 3;

  ssize_t total_send_size = 0;

  try
  {
    if (this->valid() == false)
      throw rs::exception("invalid socket handle");

    if (data == nullptr)
      throw rs::exception("invalid data buffer");

    if (size <= 0)
      throw rs::exception("invalid data size");

    if (this->type() == SOCK_DGRAM && (address == nullptr || addr_len <= 0))
      throw rs::exception("invalid address");

    // --- send data -------------------------------------------------------------
    int flag  = flags < 0 ? props_.send.base_flags : flags;
    int retry = 0;

    while (true)
    {
      int remain_size = size - total_send_size;
      int buffer_size = std::min(remain_size, props_.send.buffer_size);

      if (buffer_size <= 0)
        break;

      ssize_t send_size = 0;

      if (this->type() == SOCK_STREAM)
      {
        send_size = ::send(handle_, data + total_send_size, buffer_size, flag);
      }
      else
      {
        send_size = ::sendto(handle_, data + total_send_size, buffer_size, flag, address, addr_len);
      }

      // --- error handling ------------------------------------------------------
      if (send_size > 0)
      {
        retry = 0;
        total_send_size += send_size;
      }
      else
      {
        if (++retry >= MAX_RETRY)
          break;
      }
    }

    // --- error handling --------------------------------------------------------
    if (total_send_size != static_cast<ssize_t>(size))
    {
      // 재접속이 필요한 경우와 관련된 에러 코드
      if (errno == EPIPE ||
          errno == ECONNABORTED ||
          errno == ECONNREFUSED ||
          errno == ETIMEDOUT ||
          errno == ENETRESET ||
          errno == ECONNRESET ||
          errno == ENOTCONN ||
          errno == ENETDOWN ||
          errno == EHOSTDOWN ||
          errno == EHOSTUNREACH)
      {
        total_send_size = 0;
      }
      else
      {
        if (total_send_size > 0)
        {
          // fprintf(stderr, "send data partially failed : %ld / %ld bytes.. (%s)\n", total_send_size, size, ::strerror(errno));
        }

        total_send_size = -1;  // 사용자 측에서 에러 처리를 위해 초기화
      }

      throw rs::exception(::strerror(errno));
    }
  }
  catch (const rs::exception& e)
  {
    error_message_ = e.what();
  }

  return total_send_size;
}

inline ssize_t Socket::send(const rs::Packet& packet, int flags, const struct sockaddr* address, socklen_t addr_len) const
{
  return send(packet.data(), packet.size(), flags, address, addr_len);
}

inline ssize_t Socket::recv(void* data, size_t size, int flags, struct sockaddr* address, socklen_t* addr_len) const
{
  ssize_t read_size = 0;

  try
  {
    // --- exception -------------------------------------------------------------
    if (this->valid() == false)
      throw rs::exception("invalid socket handle");

    if (data == nullptr)
      throw rs::exception("invalid data buffer");

    if (size <= 0)
      throw rs::exception("invalid data size");

    if (this->type() == SOCK_DGRAM && (address == nullptr || addr_len == nullptr))
      throw rs::exception("invalid address");

    // --- receive data ----------------------------------------------------------
    int flag = flags < 0 ? props_.recv.base_flags : flags;

    ::memset(data, 0, size);  // clear buffer first

    if (this->type() == SOCK_STREAM)
    {
      read_size = ::recv(handle_, data, size, flag);

      // 재접속이 필요한 경우와 관련된 에러 코드
      if (read_size < 0)
      {
        if (errno == ECONNRESET ||
            errno == ECONNABORTED ||
            errno == ETIMEDOUT ||
            errno == ENETRESET ||
            errno == ENOTCONN ||
            errno == ENETDOWN ||
            errno == EHOSTDOWN ||
            errno == EHOSTUNREACH)
          read_size = 0;
      }
    }
    else
    {
      read_size = ::recvfrom(handle_, data, size, flag, address, addr_len);
    }

    // --- error handling --------------------------------------------------------
    if (read_size <= 0)
    {
      throw rs::exception(::strerror(errno));
    }
  }
  catch (const rs::exception& e)
  {
    error_message_ = e.what();
  }

  return read_size;
}

inline bool Socket::setOption(int level, int option, const void* value, socklen_t size) const
{
  if (::setsockopt(handle_, level, option, value, size) == -1)
  {
    error_message_ = ::strerror(errno);
    return false;
  }

  return true;
}

inline bool Socket::setSendTimeout(const float sec) const
{
  struct timeval timeout = float_to_timeval(sec);
  if (setOption(SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) == false)
    return false;
  else
    props_.send.timeout = timeout;

  return true;
}

inline void Socket::setSendBufferSize(int size) const
{
  if (size > 0)
    props_.send.buffer_size = size;
  else
    props_.send.buffer_size = DEFAULT_SEND_BUFFER_SIZE;
}

inline void Socket::setSendFlags(int flags) const
{
  props_.send.base_flags = flags;
}

inline bool Socket::setReceiveTimeout(const float sec) const
{
  struct timeval timeout = float_to_timeval(sec);
  if (setOption(SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) == false)
    return false;
  else
    props_.recv.timeout = timeout;

  return true;
}

inline void Socket::setReceiveFlags(int flags) const
{
  props_.recv.base_flags = flags;
}

};  // namespace domain
};  // namespace ipc
};  // namespace rs
