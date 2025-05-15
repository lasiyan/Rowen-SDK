#pragma once

#include <string>
#include <vector>

namespace rs {
namespace network {

class config
{
  constexpr static auto auto_select = "auto";      // 자동 선택
  constexpr static auto conflict    = "conflict";  // 둘 이상의 인터페이스가 존재할 때

 public:
  using Ethernets = std::vector<std::string>;

  static void release();
  static void configure();

  static Ethernets   InterfaceList();
  static std::string MacAddress(const std::string& interface = auto_select);
  static std::string IPAddress(const std::string& interface = auto_select);
  static std::string Gateway(const std::string& interface = auto_select);
  static std::string SubnetMask(const std::string& interface = auto_select);

  static std::string LastError() { return instance_.last_error_; }

 private:
  static std::string auto_select_interface(const std::string& interface = auto_select, bool retry = false);

 private:
  static config instance_;

  std::vector<std::string> available_interfaces_;
  std::string              last_error_;
};

};  // namespace network
};  // namespace rs
