#pragma once

#include <future>
#include <string>

#include "socket.hpp"

namespace rs {
namespace network {

class template_listener
{
 protected:
  virtual ~template_listener();

 public:
  /**
   * @brief stop listener
   */
  virtual void stop();

  /**
   * @brief get listener root socket
   * @return Socket class instance
   */
  class Socket& socket() { return socket_; }

  /**
   * @brief get listener port number
   * @return port number
   */
  int port() const { return socket_.address().sin_port; }

  /**
   * @brief get last error message
   * @return error message
   */
  const char* error() const { return error_.empty() ? nullptr : error_.c_str(); }

 protected:
  std::string  error_         = "";
  class Socket socket_        = {};  // listener socket
  std::mutex   listener_lock_ = {};

  // listener
  std::future<void> receiver_;
  std::atomic_bool  receiver_stop_ = true;
};

/*
----------------------------------------------------------------------------------
  Implementation
----------------------------------------------------------------------------------
*/

inline template_listener::~template_listener()
{
  stop();
}

inline void template_listener::stop()
{
  std::lock_guard<std::mutex> locker(listener_lock_);

  receiver_stop_ = true;
  if (receiver_.valid())
    receiver_.wait();

  socket_.close();
}

};  // namespace network
};  // namespace rs
