#include <arpa/inet.h>
#include <ifaddrs.h>
#include <linux/route.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <algorithm>
#include <cstring>
#include <fstream>
#include <sstream>

#include "rowen/network/config.hpp"

rs::network::config rs::network::config::instance_;

namespace rs {
namespace network {

static const char* ETHERNET_INTERFACES[] = {
  "eth", "ens", "enp", "eno"
};

static bool is_ethernet_interface(const std::string& interface)
{
  for (auto eth : ETHERNET_INTERFACES)
  {
    if (interface.find(eth) != std::string::npos)
      return true;
  }
  return false;
}

void config::release()
{
  instance_.available_interfaces_.clear();
  instance_.last_error_.clear();
}

void config::configure()
{
  release();

  struct ifaddrs* ifaddr = nullptr;

  try
  {
    // Query the network interfaces
    if (getifaddrs(&ifaddr) < 0)
      throw std::runtime_error("configure : getifaddrs : " + std::string(strerror(errno)));

    // Search for Ethernet interfaces
    for (struct ifaddrs* ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next)
    {
      if (ifa->ifa_addr == nullptr)
        continue;

      if (!is_ethernet_interface(ifa->ifa_name))
        continue;

      // Regist name (if not already registered)
      if (std::find(instance_.available_interfaces_.begin(),
                    instance_.available_interfaces_.end(),
                    ifa->ifa_name) == instance_.available_interfaces_.end())
      {
        instance_.available_interfaces_.push_back(ifa->ifa_name);
      }
    }
  }
  catch (const std::exception& e)
  {
    instance_.last_error_ = e.what();
  }

  // Free the memory allocated by getifaddrs
  if (ifaddr != nullptr)
  {
    freeifaddrs(ifaddr);
    ifaddr = nullptr;
  }
}

config::Ethernets config::InterfaceList()
{
  return instance_.available_interfaces_;
}

std::string config::MacAddress(const std::string& interface)
{
  char mac[18];
  std::memset(mac, 0, sizeof(mac));

  try
  {
    auto iface = auto_select_interface(interface);

    int fd = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0)
      throw std::runtime_error("MacAddress : invalid socket : " + std::string(strerror(errno)));

    struct ifreq ifr;
    std::strncpy(ifr.ifr_name, iface.c_str(), IFNAMSIZ - 1);
    if (ioctl(fd, SIOCGIFHWADDR, &ifr) < 0)
    {
      ::close(fd);
      throw std::runtime_error("MacAddress : ioctl : " + std::string(strerror(errno)));
    }

    ::close(fd);
    std::snprintf(mac, sizeof(mac), "%.2x:%.2x:%.2x:%.2x:%.2x:%.2x",
                  (unsigned char)ifr.ifr_hwaddr.sa_data[0],
                  (unsigned char)ifr.ifr_hwaddr.sa_data[1],
                  (unsigned char)ifr.ifr_hwaddr.sa_data[2],
                  (unsigned char)ifr.ifr_hwaddr.sa_data[3],
                  (unsigned char)ifr.ifr_hwaddr.sa_data[4],
                  (unsigned char)ifr.ifr_hwaddr.sa_data[5]);
  }
  catch (const std::exception& e)
  {
    instance_.last_error_ = e.what();
    return mac;
  }

  return mac;
}

std::string config::IPAddress(const std::string& interface)
{
  char ip[INET_ADDRSTRLEN];
  std::memset(ip, 0, sizeof(ip));

  try
  {
    auto iface = auto_select_interface(interface);

    struct ifaddrs* ifaddr = nullptr;
    if (getifaddrs(&ifaddr) < 0)
      throw std::runtime_error("IPAddress : getifaddrs : " + std::string(strerror(errno)));

    for (struct ifaddrs* ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next)
    {
      if (ifa->ifa_addr == nullptr)
        continue;

      if (std::strcmp(ifa->ifa_name, iface.c_str()) != 0)
        continue;

      if (ifa->ifa_addr->sa_family == AF_INET)
      {
        struct sockaddr_in* sa = (struct sockaddr_in*)ifa->ifa_addr;
        if (inet_ntop(AF_INET, &sa->sin_addr, ip, INET_ADDRSTRLEN) == nullptr)
          throw std::runtime_error("IPAddress : inet_ntop : " + std::string(strerror(errno)));
        break;
      }
    }

    freeifaddrs(ifaddr);
  }
  catch (const std::exception& e)
  {
    instance_.last_error_ = e.what();
    return ip;
  }

  return ip;
}

std::string config::Gateway(const std::string& interface)
{
  char gateway[INET_ADDRSTRLEN];
  std::memset(gateway, 0, sizeof(gateway));

  try
  {
    auto iface = auto_select_interface(interface);

    std::ifstream route_file("/proc/net/route");
    if (!route_file.is_open())
      throw std::runtime_error("Gateway : open : /proc/net/route");

    std::string line;
    while (std::getline(route_file, line))
    {
      std::istringstream ss(line);
      std::string        iface_name, destination, gateway_ip;

      ss >> iface_name >> destination >> gateway_ip;
      if (iface_name == iface && destination == "00000000")
      {
        unsigned long     gw;
        std::stringstream ss_gateway;
        ss_gateway << std::hex << gateway_ip;
        ss_gateway >> gw;

        struct in_addr in;
        in.s_addr = gw;
        snprintf(gateway, sizeof(gateway), "%s", inet_ntoa(in));

        break;
      }
    }

    route_file.close();
  }
  catch (const std::exception& e)
  {
    instance_.last_error_ = e.what();
    return gateway;
  }

  return gateway;
}

std::string config::SubnetMask(const std::string& interface)
{
  char subnet[INET_ADDRSTRLEN];
  std::memset(subnet, 0, sizeof(subnet));

  try
  {
    auto iface = auto_select_interface(interface);

    int fd = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0)
      throw std::runtime_error("Subnet : invalid socket : " + std::string(strerror(errno)));

    struct ifreq ifr;
    std::strncpy(ifr.ifr_name, iface.c_str(), IFNAMSIZ - 1);
    if (ioctl(fd, SIOCGIFNETMASK, &ifr) < 0)
    {
      ::close(fd);
      throw std::runtime_error("Subnet : ioctl : " + std::string(strerror(errno)));
    }

    ::close(fd);

    inet_ntop(AF_INET, &(((struct sockaddr_in*)&ifr.ifr_netmask)->sin_addr), subnet, INET_ADDRSTRLEN);
  }
  catch (const std::exception& e)
  {
    instance_.last_error_ = e.what();
    return subnet;
  }

  return subnet;
}

std::string config::auto_select_interface(const std::string& interface, bool retry)
{
  if (interface != auto_select)
    return interface;

  if (instance_.available_interfaces_.empty())
  {
    if (retry)
      throw std::runtime_error("No available interface");

    configure();
    return auto_select_interface(interface, true);
  }

  if (instance_.available_interfaces_.size() == 1)
    return instance_.available_interfaces_.front();

  if (instance_.available_interfaces_.size() > 1)
  {
    Ethernets candidates;
    for (const auto& candidate : instance_.available_interfaces_)
    {
      std::string iface = candidate;

      // remove ETHERNET_INTERFACES
      for (auto remove_key : ETHERNET_INTERFACES)
      {
        if (iface.find(remove_key) != std::string::npos)
        {
          iface.erase(0, std::strlen(remove_key));
          break;
        }
      }

      // Parse the interface number
      if (iface.empty() == false && std::isdigit(iface[0]))
      {
        std::string number = "";
        number             = iface.substr(0, iface.find_first_not_of("0123456789"));

        if (std::stoi(number) == 0)
          candidates.push_back(candidate);
      }
    }

    if (candidates.size() == 1)
      return candidates.front();
    else
      throw std::runtime_error("Two or more `0` interfaces exist");
  }

  throw std::runtime_error("No available interface");
}

}  // namespace network
}  // namespace rs
