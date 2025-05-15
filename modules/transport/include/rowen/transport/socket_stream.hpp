#include "socket_base.hpp"

namespace rs {
namespace transport {

class StreamSocket : public internal::SocketBase
{
 public:
  template <typename T>
  bool bind(const T& port_or_domain_path);

  bool listen(int backlog = 5);

  StreamSocket accept();

  bool connect(const sockaddr* socket_address, socklen_t address_len);

  ssize_t send(const uint8_t* data, size_t size, int flags = -1);

  ssize_t recv(void* buffer, size_t size, int flags = -1);
};

};  // namespace transport
};  // namespace rs
