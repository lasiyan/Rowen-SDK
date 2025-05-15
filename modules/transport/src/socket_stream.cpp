#include "rowen/transport/socket_stream.hpp"

namespace rs {
namespace transport {

bool StreamSocket::bind(const sockaddr* socket_address, socklen_t address_len)
{
  std::lock_guard<std::recursive_mutex> locker(mutex_);

  if (::bind(socket_fd_, socket_address, address_len) == -1)
  {
    setError("bind");
    return false;
  }

  if (this->valid() == false)
    return false;

  if (::bind(socket_fd_, socket_address, address_len) == -1)
  {
    setErrorMessage("bind");
    return false;
  }

  return true;
}

}  // namespace transport
}  // namespace rs
