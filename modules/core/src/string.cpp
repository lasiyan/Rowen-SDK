#include <cstdarg>
#include <vector>

#include "rowen/core/function.hpp"

namespace rs {

std::string& ltrim(std::string& s, const char* t)
{
  s.erase(0, s.find_first_not_of(t));
  return s;
}
std::string& rtrim(std::string& s, const char* t)
{
  s.erase(s.find_last_not_of(t) + 1);
  return s;
}
std::string& trim(std::string& s, const char* t)
{
  return ltrim(rtrim(s, t), t);
}

// string
std::string format(const char* fmt, ...)
{
  va_list va_args;
  va_start(va_args, fmt);
  va_list va_args_copy;
  va_copy(va_args_copy, va_args);
  const int result_length = std::vsnprintf(NULL, 0, fmt, va_args_copy);
  va_end(va_args_copy);
  std::vector<char> result(result_length + 1);
  std::vsnprintf(result.data(), result.size(), fmt, va_args);
  va_end(va_args);
  return std::string(result.data(), result_length);
}

}  // namespace rs
