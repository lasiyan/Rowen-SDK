#pragma once

#include <array>

#include "logger/logger_console.hpp"
#include "logger/logger_file.hpp"
#ifdef RS_WITH_DATABASE_LOGGER
  #include "logger/logger_websvc.hpp"
#endif

namespace rs {

enum LoggerTarget
{
  CONSOLE,
  FILE,
#ifdef RS_WITH_DATABASE_LOGGER
  WEBSVC,
#endif
  LoggerTargetMax,
};

class Logger
{
  /* --------------------------------------------------------------------------- */
  /**
   * @brief 사용: logger.log(props, "Hello World");
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

    if (loggers_[LoggerTarget::CONSOLE])
      console.logImpl(props, &context, RS_LOGGER_FORMAT_CAST(fmt), args...);

    if (loggers_[LoggerTarget::FILE])
      file.logImpl(props, &context, RS_LOGGER_FORMAT_CAST(fmt), args...);

#ifdef RS_WITH_DATABASE_LOGGER
    if (loggers_[LoggerTarget::WEBSVC])
      websvc.logImpl(props, &context, RS_LOGGER_FORMAT_CAST(fmt), args...);
#endif
  }

  /* --------------------------------------------------------------------------- */
  /**
   * @brief 사용: logger.[LOG_LEVEL]("Hello World");
   * @param fmt 로그 출력 포맷
   * @param args 로그 출력 인자
   */
  RS_LOGGER_WRAPPING_LIST()

 private:
  template <typename... Args>  // for RS_LOGGER_WRAPPING_LIST
  void logImpl(LoggerLevel level, RS_LOGGER_FORMAT_TYPE fmt, Args... args)
  {
    log(level, nullptr, 0, fmt, args...);
  }

  /* --------------------------------------------------------------------------- */
  /**
   * @brief 사용: logger.log(Level, __FILE__, __LINE__, "Hello World");
   * @param level 로그 출력 레벨
   * @param location 파일 위치
   * @param line 라인 번호
   * @param fmt 로그 출력 포맷
   * @param args 로그 출력 인자
   */
 public:
  template <typename... Args>
  void log(LoggerLevel level, const char* location, const int line, RS_LOGGER_FORMAT_TYPE fmt, Args... args)
  {
    LoggerBase::Context context;
    context.footer.file = location;
    context.footer.line = line;

    if (loggers_[LoggerTarget::CONSOLE])
      console.logImpl(level, &context, RS_LOGGER_FORMAT_CAST(fmt), args...);

    if (loggers_[LoggerTarget::FILE])
      file.logImpl(level, &context, RS_LOGGER_FORMAT_CAST(fmt), args...);

#ifdef RS_WITH_DATABASE_LOGGER
    if (loggers_[LoggerTarget::WEBSVC])
      websvc.logImpl(level, &context, RS_LOGGER_FORMAT_CAST(fmt), args...);
#endif
  }

  /* --------------------------------------------------------------------------- */

 private:
  template <typename Func, typename... Args>
  void setProperty(Func func, Args&&... args)
  {
    for (auto& logger : loggers_)
    {
      if (logger)
        (logger.get()->*func)(std::forward<Args>(args)...);
    }
  }

 public:
  void activate(LoggerTarget target);
  void deactivate(LoggerTarget target);

  LoggerProperty getProperty(LoggerTarget target = FILE) const;

  void        setLoggerLevel(LoggerLevel level);
  LoggerLevel getLoggerLevel(LoggerTarget target = FILE) const;

  void        setCategory(std::string category) { setProperty(&LoggerBase::setCategory, category); }
  std::string getCategory(LoggerTarget target = FILE) const { return loggers_[target]->getCategory(); }

  void setHeaderDate(bool enable) { setProperty(&LoggerBase::setHeaderDate, enable); }
  bool getUsingHeaderDate(LoggerTarget target = FILE) const { return loggers_[target]->getUsingHeaderDate(); }

  void setMicroResolution(bool enable) { setProperty(&LoggerBase::setMicroResolution, enable); }
  bool getUsingMicroResolution(LoggerTarget target = FILE) const { return loggers_[target]->getUsingMicroResolution(); }

  // void setColorLogging(bool enable) { setProperty(&LoggerBase::setColorLogging, enable); }
  // bool getColorLogging(LoggerTarget target = FILE) const { return loggers_[target]->getColorLogging(); }

  void        setDirectory(std::string directory) { file.setDirectory(directory); }
  std::string getDirectory() const { return file.getDirectory(); }
  void        resetDirectory() { file.resetDirectory(); }

 private:
  std::array<std::unique_ptr<LoggerBase>, LoggerTargetMax> loggers_ = {
    std::make_unique<ConsoleLogger>(),
    std::make_unique<FileLogger>(),
#ifdef RS_WITH_DATABASE_LOGGER
    std::make_unique<WebsvcLogger>(),
#endif
  };

 public:
  ConsoleLogger& console = static_cast<ConsoleLogger&>(*loggers_[LoggerTarget::CONSOLE]);
  FileLogger&    file    = static_cast<FileLogger&>(*loggers_[LoggerTarget::FILE]);
#ifdef RS_WITH_DATABASE_LOGGER
  WebsvcLogger& websvc = static_cast<WebsvcLogger&>(*loggers_[LoggerTarget::WEBSVC]);
#endif
};

};  // namespace rs

inline rs::Logger         logger;
inline rs::ConsoleLogger& console = logger.console;

// clang-format off
#define logger_fatal(fmt, ...)        (logger.log(rs::LoggerLevel::FATAL, RS_FILENAME, __LINE__, (RS_LOGGER_FORMAT_TYPE)fmt, ##__VA_ARGS__))
#define logger_error(fmt, ...)        (logger.log(rs::LoggerLevel::ERROR, RS_FILENAME, __LINE__, (RS_LOGGER_FORMAT_TYPE)fmt, ##__VA_ARGS__))
#define logger_warn(fmt, ...)         (logger.log(rs::LoggerLevel::WARN,  RS_FILENAME, __LINE__, (RS_LOGGER_FORMAT_TYPE)fmt, ##__VA_ARGS__))
#define logger_info(fmt, ...)         (logger.log(rs::LoggerLevel::INFO,  RS_FILENAME, __LINE__, (RS_LOGGER_FORMAT_TYPE)fmt, ##__VA_ARGS__))
#define logger_debug(fmt, ...)        (logger.log(rs::LoggerLevel::DEBUG, RS_FILENAME, __LINE__, (RS_LOGGER_FORMAT_TYPE)fmt, ##__VA_ARGS__))
#define logger_trace(fmt, ...)        (logger.log(rs::LoggerLevel::TRACE, RS_FILENAME, __LINE__, (RS_LOGGER_FORMAT_TYPE)fmt, ##__VA_ARGS__))
#define logger_errno(_TITLE_, _STR_)   logger.error("%s : %s (%d, %s)", _TITLE_, _STR_, errno, strerror(errno));
// clang-format on
