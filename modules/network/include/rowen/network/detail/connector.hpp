#pragma once

#include <rowen/network/detail/socket.hpp>
#include <string>

namespace rs {
namespace network {

class template_connector
{
 protected:
  virtual ~template_connector();

 public:
  /**
   * @brief get socket
   * @return Socket class instance
   */
  class Socket& socket() { return socket_; }

  /**
   * @brief get socket address
   * @return socket address
   */
  const char* address() const { return server_ipv4_.c_str(); }

  /**
   * @brief get socket port
   * @return socket port
   */
  int port() const { return server_port_; }

  /**
   * @brief get error message
   * @return error message
   */
  const char* error() const { return error_.empty() ? nullptr : error_.c_str(); }

 protected:
  std::string error_ = "";

  // server information
  int         server_port_ = 0;
  std::string server_ipv4_ = "";

  // connector socket
  class Socket socket_ = {};
};

/*
----------------------------------------------------------------------------------
  Implementation
----------------------------------------------------------------------------------
*/

inline template_connector::~template_connector()
{
  socket_.close();
}

};  // namespace network
};  // namespace rs
