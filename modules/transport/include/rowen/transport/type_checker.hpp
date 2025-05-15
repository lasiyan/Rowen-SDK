#pragma once

#include <arpa/inet.h>
#include <sys/un.h>

#include <cassert>
#include <string>

namespace rs {

inline bool verify_port(const int port)
{
  return (port >= 0 && port <= 65535);
}
inline void assert_port(const int port)
{
  assert(verify_port(port) && "invalid port number");
}

inline bool verify_unix_domain(const std::string& domain_path)
{
  return (!domain_path.empty() && domain_path.length() < sizeof(sockaddr_un::sun_path));
}
inline void assert_unix_domain(const std::string& domain_path)
{
  assert(verify_unix_domain(domain_path) && "invalid unix domain path");
}

inline bool verify_ipv4(const std::string& ip_address)
{
  return (!ip_address.empty() && ip_address.length() < INET_ADDRSTRLEN);
}
inline void assert_ipv4(const std::string& ip_address)
{
  assert(verify_ipv4(ip_address) && "invalid ipv4 address");
}

};  // namespace rs
