#pragma once

#include <exception>
#include <string>

namespace rs {

class exception : public std::exception
{
 public:
  exception(const std::string& message) : message_(message) {}
  virtual ~exception() noexcept {}

  virtual const char* what() const noexcept override { return message_.c_str(); }

 protected:
  std::string message_;
};

};  // namespace rs
