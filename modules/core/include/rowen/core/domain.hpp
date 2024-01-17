#ifndef ROWEN_SDK_CORE_DOMAIN_HPP
#define ROWEN_SDK_CORE_DOMAIN_HPP

#include <sys/socket.h>
#include <sys/un.h>

#include <string>

#include "function.hpp"

namespace rs {
namespace domain {

static constexpr char INSTALL_PATH[]   = "/tmp";
static constexpr char INSTALL_PREFIX[] = "rowen_domain";
static constexpr int  BUFFER_SIZE      = 8192;

class exception : public std::exception
{
  std::string message_;
  bool        debugging_ = false;

 public:
  /**
  @param message 예외 메시지 정의
  @param debugging 디버깅용 로그 (디버깅 모드 또는 로그 상태에서 출력)
  @param with_errno with errno (errno가 있는 경우 출력)
  @param _file file name
  @param _line line number
  @param _class class name
  @param _function function name
  */
  exception(const std::string& message, bool debugging, bool with_errno,
            const char* _file, int _line, const char* _class, const char* _function);

  virtual ~exception() throw() {}

  virtual const char* what() const throw() { return message_.c_str(); }
  bool                debugging() const { return debugging_; }

 private:
  void format_message(const std::string& message, bool with_errno,
                      const char* _file, int _line, const char* _class, const char* _function);
};

class socket
{
  constexpr static auto REX_INVALID_SOCKET = -1;

 public:
  virtual bool prepare(const std::string& domain = "") = 0;

 public:
  void release();
  void timeout(unsigned int sec);

  ssize_t send(void* buffer, size_t send_size, int flags = 0);
  ssize_t recv(void* buffer, size_t buffer_size, int flags = 0);

  int                socket_num() const { return socket_; }
  const sockaddr_un& socket_addr() const { return socket_addr_; }
  const char*        domain() const { return domain_.c_str(); }

 protected:
  socket(int type, const std::string& domain = "");
  virtual ~socket();

  static void release(socket& socket);
  void        assert_socket();
  void        set_domain(const std::string& domain);

 protected:
  int         socket_      = REX_INVALID_SOCKET;
  sockaddr_un socket_addr_ = {};
  std::string domain_      = "";
  int         type_        = SOCK_DGRAM;
};

class datagram_connector : public socket
{
 public:
  datagram_connector(const std::string& domain = "");
  ~datagram_connector();

  bool prepare(const std::string& domain = "") override;
};

class datagram_listener : public socket
{
 public:
  datagram_listener(const std::string& domain = "");
  ~datagram_listener();

  bool prepare(const std::string& domain = "") override;
};

};  // namespace domain
};  // namespace rs

#define REX_DOMAIN_THROW_EXCEPTION(msg, errno) \
  throw rs::domain::exception(msg, false, errno, __FILENAME__, __LINE__, __CLASS__, __METHOD__)

#define REX_DOMAIN_THROW_EXCEPTION_DEBUG(msg, errno) \
  throw rs::domain::exception(msg, true, errno, __FILENAME__, __LINE__, __CLASS__, __METHOD__)

#endif  // ROWEN_SDK_CORE_DOMAIN_HPP
