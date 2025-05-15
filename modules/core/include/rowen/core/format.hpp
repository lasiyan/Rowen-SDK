#pragma once

#include <string>

// #define RS_FORMAT_C_STYLE

#ifndef RS_FORMAT_C_STYLE
#else
  #include <cstdarg>
  #include <cstdio>
  #include <vector>
#endif

namespace rs {

#ifndef RS_FORMAT_C_STYLE

template <typename... Args>
inline std::string format(const char* fmt, Args&&... args)
{
  if constexpr (sizeof...(Args) == 0)
  {
    return std::string(fmt);
  }
  else
  {
    int size = std::snprintf(nullptr, 0, fmt, std::forward<Args>(args)...);
    if (size <= 0)
      return {};

    std::string result(size, '\0');
    std::snprintf(result.data(), size + 1, fmt, std::forward<Args>(args)...);
    return result;
  }
}

#else

inline std::string format(const char* fmt, ...)
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

#endif

};  // namespace rs
