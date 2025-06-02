#pragma once

#include <rowen/logger/detail/logger_base.hpp>

namespace rs {

class ConsoleLogger : public LoggerBase
{
  friend class Logger;

 public:
  ConsoleLogger() : LoggerBase()
  {
    default_props_.using_header_date      = false;
    default_props_.using_micro_resolution = false;
    default_props_.raw_logging            = false;
    default_props_.ignore_level           = false;
    default_props_.styled_logging         = true;
  }
  ~ConsoleLogger() = default;

  /* --------------------------------------------------------------------------- */
  /**
   * @brief 사용: logger.console.log(props, "Hello World");
   * @details 속성을 지정하여 로그를 출력합니다.
   * @param props 로그 출력 속성 (1회성)
   * @param fmt 로그 출력 포맷
   * @param args 로그 출력 인자
   */
 public:
  template <typename... Args>
  void log(const LoggerProperty& props, RS_LOGGER_FORMAT_TYPE fmt, Args... args)
  {
    LoggerBase::Context context;
    logImpl(props, &context, RS_LOGGER_FORMAT_CAST(fmt), args...);
  }

 private:
  template <typename... Args>
  void logImpl(LoggerProperty props, LoggerBase::Context* context, const char* fmt, Args... args)
  {
    updateInheritProperties(props);
    __logImpl(props.level, props, context, fmt, args...);
  }

  /* --------------------------------------------------------------------------- */
  /**
   * @brief 사용: logger.console.[LOG_LEVEL]("Hello World");
   * @param fmt 로그 출력 포맷
   * @param args 로그 출력 인자
   */
  RS_LOGGER_WRAPPING_LIST()

 private:
  template <typename... Args>  // for RS_LOGGER_WRAPPING_LIST
  void logImpl(LoggerLevel level, const char* fmt, Args... args)
  {
    LoggerBase::Context context;
    __logImpl(level, default_props_, &context, fmt, args...);
  }

  template <typename... Args>  // Logger클래스에서 호출될 때 사용
  void logImpl(LoggerLevel level, LoggerBase::Context* context, const char* fmt, Args... args)
  {
    __logImpl(level, default_props_, context, fmt, args...);
  }

  /* --------------------------------------------------------------------------- */
  template <typename... Args>
  void __logImpl(LoggerLevel level, const LoggerProperty& props, LoggerBase::Context* context, const char* fmt, Args... args)
  {
    if (assertLevel(level, &props) == false)
      return;

    if (context == nullptr)
      return;

    // Fill context (if not exist ..)
    LoggerBase::setHeaderContext(level, props, context);
    LoggerBase::setBodyContext(props, context, fmt, std::forward<Args>(args)...);
    LoggerBase::setFooterContext(props, context);

    // Generate Content
    auto content = LoggerBase::generateContent(level, props, context, true);

    // Print Content
    printf("%s", content.c_str());
  }
};

};  // namespace rs
