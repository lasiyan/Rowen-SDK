#include "rowen/core/domain.hpp"

#include <unistd.h>

namespace rs {
namespace domain {
exception::exception(const std::string& message, bool debugging, bool with_errno,
                     const char* file, int line,
                     const char* class_name, const char* function_name)
    : debugging_(debugging)
{
  format_message(message, with_errno, file, line, class_name, function_name);
}

void exception::format_message(const std::string& message, bool with_errno,
                               const char* file, int line,
                               const char* class_name, const char* function_name)
{
  message_ = format("domain::exception : %s", message.c_str());

  if (file && line)
    message_.append(format("  (%s:%d)", file, line));

  if (class_name && function_name)
    message_.append(format(" in function '%s::%s'", class_name, function_name));

  if (with_errno)
    message_.append(format("\n error: %s", strerror(errno)));
}

// --------------------------------------------------------------------------------
socket::socket(int type, const std::string& domain)
    : socket_(REX_INVALID_SOCKET), domain_(domain), type_(type)
{
  socket_ = ::socket(AF_UNIX, type, 0);
}

socket::~socket()
{
  release();
}

void socket::release(socket& socket)
{
  if (socket.socket_ != REX_INVALID_SOCKET)
  {
    ::close(socket.socket_);
    socket.socket_ = REX_INVALID_SOCKET;
  }
}

void socket::release()
{
  release(*this);
}

void socket::timeout(unsigned int sec)
{
  assert_socket();

  struct timeval timeout = { sec, 0 };

  if (setsockopt(socket_, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout)) < 0)
    REX_DOMAIN_THROW_EXCEPTION("setsockopt() (SO_RCVTIMEO)", true);

  if (setsockopt(socket_, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeout)) < 0)
    REX_DOMAIN_THROW_EXCEPTION("setsockopt() (SO_SNDTIMEO)", true);
}

ssize_t socket::send(void* buffer, size_t send_size, int flags)
{
  assert_socket();

  ssize_t res = 0;

  if (type_ == SOCK_DGRAM)
  {
    auto length = sizeof(socket_addr_);

    res = ::sendto(socket_, buffer, send_size, flags, (struct sockaddr*)&socket_addr_, length);
  }
  else
  {
    res = ::send(socket_, buffer, send_size, flags);
  }

  // result
  if (res == 0)
  {
    return 0;
  }
  else if (res < 0)
  {
    // 소켓이 논블록킹으로 설정되어 있을 경우
    if (errno == EAGAIN || errno == EWOULDBLOCK)
      return res;
    // 서버 접근이 불가능한 경우
    else if (errno == ECONNREFUSED)
    {
      REX_DOMAIN_THROW_EXCEPTION_DEBUG("server is not running", true);
    }
    // 서버가 종료된 경우
    else if (errno == ENOENT)
    {
      REX_DOMAIN_THROW_EXCEPTION_DEBUG("server is closing", true);
    }
    else
    {
      REX_DOMAIN_THROW_EXCEPTION("send()", true);
    }
  }

  return res;
}

ssize_t socket::recv(void* buffer, size_t buffer_size, int flags)
{
  assert_socket();

  ssize_t res = 0;

  if (type_ == SOCK_DGRAM)
  {
    auto length = sizeof(socket_addr_);

    res = ::recvfrom(socket_, buffer, buffer_size, flags, (struct sockaddr*)&socket_addr_, (socklen_t*)&length);
  }
  else
  {
    res = ::recv(socket_, buffer, buffer_size, flags);
  }

  // result
  if (res == 0 && type_ == SOCK_STREAM)
  {
    // 클라이언트가 연결을 끊은 경우
    return 0;
  }
  else if (res < 0)
  {
    // 타임아웃인 경우 (재시도)
    if (errno == EAGAIN || errno == EWOULDBLOCK)
      return res;
    else
      REX_DOMAIN_THROW_EXCEPTION("recv()", true);
  }

  return res;
}

void socket::assert_socket()
{
  if (socket_ == REX_INVALID_SOCKET)
    REX_DOMAIN_THROW_EXCEPTION("socket is not created", true);

  if (domain_.empty())
    REX_DOMAIN_THROW_EXCEPTION("domain is empty", false);
}

void socket::set_domain(const std::string& domain_name)
{
  if (domain_name.empty())
    REX_DOMAIN_THROW_EXCEPTION("empty argument (domain)", false);

  if (domain_name.find(domain::INSTALL_PATH) == std::string::npos)
    domain_ = rs::format("%s/%s.%s", INSTALL_PATH, INSTALL_PREFIX, domain_name.c_str());
  else
    domain_ = domain_name;

  socket_addr_.sun_family = AF_UNIX;
  strncpy(socket_addr_.sun_path, domain_.c_str(), sizeof(socket_addr_.sun_path) - 1);
}

// --------------------------------------------------------------------------------
datagram_connector::datagram_connector(const std::string& domain)
    : socket(SOCK_DGRAM, domain)
{
}

datagram_connector::~datagram_connector()
{
}

bool datagram_connector::prepare(const std::string& domain)
{
  set_domain(domain);
  return true;
}

// --------------------------------------------------------------------------------
datagram_listener::datagram_listener(const std::string& domain)
    : socket(SOCK_DGRAM, domain)
{
}

datagram_listener::~datagram_listener()
{
  ::unlink(domain_.c_str());
}

bool datagram_listener::prepare(const std::string& domain)
{
  set_domain(domain);

  // reuse address
  ::unlink(domain_.c_str());

  int length = sizeof(socket_addr_);

  if (::bind(socket_, (struct sockaddr*)&socket_addr_, length) < 0)
    REX_DOMAIN_THROW_EXCEPTION("bind()", true);

  return true;
}

// --------------------------------------------------------------------------------

//
};  // namespace domain
};  // namespace rs
