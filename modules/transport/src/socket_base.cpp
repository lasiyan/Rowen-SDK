#include "rowen/transport/socket_base.hpp"

#include <unistd.h>

#include <cstring>

#include "rowen/core/format.hpp"
#include "rowen/transport/type_checker.hpp"

namespace rs {
namespace transport {
namespace internal {

#define RETURN_IF_ERROR(pass_condition, function) \
  if ((pass_condition) == false)                  \
  {                                               \
    setError(function);                           \
    return false;                                 \
  }

#define RETURN_IF_ERROR_MESSAGE(pass_condition, function, message) \
  if ((pass_condition) == false)                                   \
  {                                                                \
    setError(function, message);                                   \
    return false;                                                  \
  }

#define RETURN_IF_INVALID_SOCKET(function) \
  RETURN_IF_ERROR_MESSAGE(this->valid(), function, "invalid socket handle")

template <typename AddressType>
SocketBase<AddressType>::~SocketBase()
{
  close();
}

template <typename AddressType>
bool SocketBase<AddressType>::open(int domain, int socket_type, int protocol)
{
  std::lock_guard<std::recursive_mutex> locker(mutex_);

  close();

  // Create socket
  socket_fd_     = ::socket(domain, socket_type, protocol);
  socket_type_   = socket_type;
  socket_domain_ = domain;

  // Check socket creation
  RETURN_IF_ERROR(socket_fd_ != INVALID_SOCKET, "open");

  // Caching socket domain, but sockaddr_ will be set in bind() or connect()

  return true;
}

template <typename AddressType>
void SocketBase<AddressType>::close()
{
  std::lock_guard<std::recursive_mutex> locker(mutex_);

  if (socket_fd_ != -1)
  {
    ::close(socket_fd_);
    socket_fd_ = -1;
  }

  error_.clear();
  std::memset(&sockaddr_, 0, sizeof(sockaddr_));
}

template <typename AddressType>
bool SocketBase<AddressType>::shutdown(int how)
{
  std::lock_guard<std::recursive_mutex> locker(mutex_);

  RETURN_IF_INVALID_SOCKET("shutdown");

  if (::shutdown(socket_fd_, how) == -1)
  {
    setError("shutdown");
    return false;
  }

  return true;
}

template <typename AddressType>
bool SocketBase<AddressType>::valid() const
{
  std::lock_guard<std::recursive_mutex> locker(mutex_);

  return socket_fd_ != INVALID_SOCKET;
}

template <typename AddressType>
bool SocketBase<AddressType>::bind(const int port)
{
  static_assert(std::is_same<AddressType, sockaddr_in>::value, "bind() only supports IPv4");

  std::lock_guard<std::recursive_mutex> locker(mutex_);

  RETURN_IF_INVALID_SOCKET("bind");
  RETURN_IF_ERROR_MESSAGE(verify_port(port), "bind", "invalid port");
  RETURN_IF_ERROR_MESSAGE(socket_domain_ == AF_INET, "bind", "invalid socket domain");

  // Set socket address
  sockaddr_in addr{};
  addr.sin_family      = socket_domain_;
  addr.sin_addr.s_addr = ::htonl(INADDR_ANY);  // Bind to all interfaces
  addr.sin_port        = ::htons(port);

  std::memset(&sockaddr_, 0, sizeof(sockaddr_));
  std::memcpy(&sockaddr_, &addr, sizeof(addr));

  // Bind socket
  if (::bind(socket_fd_, (struct sockaddr*)&sockaddr_, sizeof(sockaddr_)) == -1)
  {
    setError("bind");
    return false;
  }

  return true;
}

template <typename AddressType>
bool SocketBase<AddressType>::bind(const std::string& domain_path)
{
  static_assert(std::is_same<AddressType, sockaddr_un>::value, "bind() only supports Unix domain");

  std::lock_guard<std::recursive_mutex> locker(mutex_);

  RETURN_IF_INVALID_SOCKET("bind");
  RETURN_IF_ERROR_MESSAGE(verify_unix_domain(domain_path), "bind", "invalid unix domain path");
  RETURN_IF_ERROR_MESSAGE(socket_domain_ == AF_UNIX, "bind", "invalid socket domain");

  // Remove existing socket file
  if (::unlink(domain_path.c_str()) == -1 && errno != ENOENT)
  {
    setError("bind(unlink)");
    return false;
  }

  // Set socket address
  sockaddr_un addr{};
  addr.sun_family = socket_domain_;
  std::strncpy(addr.sun_path, domain_path.c_str(), sizeof(addr.sun_path) - 1);

  std::memset(&sockaddr_, 0, sizeof(sockaddr_));
  std::memcpy(&sockaddr_, &addr, sizeof(addr));

  // Bind socket
  if (::bind(socket_fd_, (struct sockaddr*)&sockaddr_, sizeof(sockaddr_)) == -1)
  {
    setError("bind");
    return false;
  }

  return true;
}

template <typename AddressType>
bool SocketBase<AddressType>::listen(int backlog)
{
  std::lock_guard<std::recursive_mutex> locker(mutex_);

  RETURN_IF_INVALID_SOCKET("listen");
  RETURN_IF_ERROR_MESSAGE(backlog > 0, "listen", "invalid backlog value");

  // Listen on socket
  if (::listen(socket_fd_, backlog) == -1)
  {
    setError("listen");
    return false;
  }

  return true;
}

template <typename AddressType>
void SocketBase<AddressType>::setError(const char* function_name, const std::string& user_message) const
{
  std::lock_guard<std::recursive_mutex> locker(mutex_);

  assert(function_name != nullptr);

  error_ = rs::format("%s : %s", function_name, user_message.empty() ? ::strerror(errno) : user_message.c_str());
}

}  // namespace internal
}  // namespace transport
}  // namespace rs
