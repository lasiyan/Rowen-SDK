#pragma once

#include <string>
#include <vector>

#include "typedef_status.hpp"

// #define RS_RESPONSE_ALLOW_STRING_OPERATOR

#ifndef RS_RESPONSE_ALLOW_STRING_OPERATOR
  #define RS_EXPLICT_BOOL    template <typename T, typename std::enable_if_t<std::is_same_v<T, bool>>* = nullptr>
  #define RS_EXPLICT_BOOL_EX RS_EXPLICT_BOOL explicit
#else
  #define RS_EXPLICT_BOOL
  #define RS_EXPLICT_BOOL_EX explicit
#endif

namespace Json {
class Value;
};  // namespace Json

namespace rs {

class structResponse
{
 protected:
  template <typename... Args>
  static std::string format_string(const char* fmt, Args&&... args)
  {
    if constexpr (sizeof...(args) == 0)
    {
      return fmt ? std::string(fmt) : "";
    }
    else
    {
      int size = std::snprintf(nullptr, 0, fmt, std::forward<Args>(args)...);
      if (size <= 0)
        return "";

      std::vector<char> buffer(size + 1);
      std::snprintf(buffer.data(), buffer.size(), fmt, std::forward<Args>(args)...);
      return std::string(buffer.data(), size);
    }
  }

 public:
  int         status  = rssOK;                            // response status
  std::string message = rss_global_state_string(status);  // response status message

  // Constructor : ex. response_t res(rssOK, "success");
  structResponse() = default;
  structResponse(int status, const std::string& log = "") { set(status, log.empty() ? rss_global_state_string(status) : log); }

#ifdef RS_RESPONSE_ALLOW_STRING_OPERATOR
  // Constructor : ex. response_t res = "error message"
  structResponse(const char* log) : structResponse(rssUndefined, log) {}
  structResponse(const std::string& log) : structResponse(log.c_str()) {}
#endif

  // Constructor : ex. response_t res(true); (only for bool)
  RS_EXPLICT_BOOL_EX structResponse(bool res)
  {
    auto s = res ? rssOK : rssUndefined;
    set(s, rss_global_state_string(s));
  }

  // Copy operator : ex. response_t res = rssOK;  OR  response_t res = true;
  // clang-format off
  structResponse& operator=(int status) { return set(status, rss_global_state_string(status)); }
  RS_EXPLICT_BOOL structResponse& operator=(bool res) { auto s = res ? rssOK : rssUndefined; return set(s, rss_global_state_string(s)); }
#ifdef RS_RESPONSE_ALLOW_STRING_OPERATOR
  structResponse& operator=(const char* log) { return set(rssUndefined, log); }
  structResponse& operator=(const std::string& log) { return set(rssUndefined, log); }
#endif
  // clang-format on

  // Comparison operator : ex. if (res == rssOK) { ... }  OR  if (res == true) { ... }
  bool     operator==(bool result) const { return success() == result; }
  bool     operator==(int code) const { return status == code; }
  explicit operator bool() const { return success(); }
  explicit operator StatusCode() const { return static_cast<StatusCode>(status); }

  // operator() : ex. return res(rssOK, "success");
  structResponse& operator()(int status, const std::string& log = "") { return set(status, log.empty() ? rss_global_state_string(status) : log); }

  /**
   * @brief Check response state is success
   */
  bool success(bool strict = true) const { return rss_global_state_is_success(status, strict); }

  /**
   * @brief Get response status message
   * @return status message (const char*)
   */
  const char* c_str() const { return message.c_str(); }

  /**
   * @brief Set response
   * @details Overwrite status and status message
   */
  virtual structResponse& set(int status, const std::string& log)
  {
    this->status  = status;
    this->message = log;
    return *this;
  }

  /**
   * @brief Set response with formatter
   */
  template <typename... Args>
  structResponse& setFmt(int status, const char* fmt, Args&&... args)
  {
    return set(status, format_string(fmt, std::forward<Args>(args)...));
  }
  template <typename... Args>
  structResponse& set(int status, const char* fmt, Args&&... args)
  {
    return setFmt(status, fmt, std::forward<Args>(args)...);
  }

  /**
   * @brief Reset response
   * @details Reset status and status message
   */
  virtual structResponse& reset() { return set(rssOK, rss_global_state_string(rssOK)); }
};

// --- response template ---------------------------------------------------------
// Helper to disable template instantiation for void types
template <typename T, typename Enable = void>
struct NotVoid : std::true_type
{
};

template <typename T>
struct NotVoid<T, typename std::enable_if<std::is_same<T, void>::value>::type> : std::false_type
{
};

template <typename T>
struct response : structResponse
{
  static_assert(NotVoid<T>::value, "response<void> is not allowed");

  using structResponse::structResponse;
  using structResponse::operator=;
  T content = T{};

  response() = default;
  response(int status, const std::string& log, const T& content)
  {
    set(status, log.empty() ? rss_global_state_string(status) : log, content);
  }

  /**
   * @brief Copy operator
   * @details ex. response_int res_i = response_t(rssOK, "success");
   */
  response& operator=(const structResponse& res)
  {
    this->status  = res.status;
    this->message = res.message;
    return *this;
  }

  /**
   * @brief operator() defined
   * @details ex. return response(rssOK, "success");
   */
  response& operator()(int status, const std::string& log) { return set(status, log.empty() ? rss_global_state_string(status) : log); }
  response& operator()(int status, const std::string& log, const T& content) { return set(status, log.empty() ? rss_global_state_string(status) : log, content); }

  /**
   * @brief Set response with content
   * @return response object
   */
  response& set(int status, const std::string& log) override
  {
    structResponse::set(status, log);
    return *this;
  }

  response& set(int status, const std::string& log, const T& content)
  {
    structResponse::set(status, log);
    this->content = content;
    return *this;
  }

  template <typename... Args>
  response& setFmt(int status, const char* fmt, Args&&... args)
  {
    structResponse::setFmt(status, fmt, std::forward<Args>(args)...);
    return *this;
  }

  /**
   * @brief Set content only
   */
  response& fill(const T& content)
  {
    this->content = content;
    return *this;
  }
};

// typedef alias
using response_t      = structResponse;
using response_void   = structResponse;
using response_int    = response<int>;
using response_uint   = response<uint>;
using response_char   = response<char>;
using response_string = response<std::string>;
using response_bool   = response<bool>;
using response_float  = response<float>;
using response_double = response<double>;
using response_long   = response<long>;
using response_ulong  = response<ulong>;
using response_json   = response<Json::Value>;

};  // namespace rs
