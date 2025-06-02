#pragma once

#include <cstdint>
#include <optional>
#include <rowen/core/format.hpp>
#include <rowen/logger/detail/logger_config.hpp>  // IWYU pragma: export
#include <rowen/logger/detail/logger_styleDef.hpp>
#include <string>

#define RS_LOGGER_DEFAULT_CATEGORY  "system"
#define RS_LOGGER_DEFAULT_DIRECTORY "log"
#define RS_LOGGER_BACKUP_DIRECTORY  "backup"
#define RS_LOGGER_FORMAT_EXTEND     true

#if (RS_LOGGER_FORMAT_EXTEND)
  #define RS_LOGGER_FORMAT_TYPE    const std::string&
  #define RS_LOGGER_FORMAT_CAST(X) X.c_str()
#else
  #define RS_LOGGER_FORMAT_TYPE    const char*
  #define RS_LOGGER_FORMAT_CAST(X) (X)
#endif

namespace rs {

enum class LoggerLevel : int
{
  SILENT,  // 모든 로그 끄기
  FATAL,   // 운영 간 치명적인 오류 및 예외 상황
  ERROR,   // 기능 동작 실패
  WARN,    // 기능 동작은 실패했으나 운영에 영향 없음 또는 기능은 성공했으나, 예상치 못한 상황
  INFO,    // 일반 로그
  DEBUG,   // 디버깅 관련 로그 (개발자에게 필요한 로그)
  TRACE,   // 상세 로그 정보
};

#define RS_SILENT_LOG rs::LoggerLevel::SILENT
#define RS_FATAL_LOG  rs::LoggerLevel::FATAL
#define RS_ERROR_LOG  rs::LoggerLevel::ERROR
#define RS_WARN_LOG   rs::LoggerLevel::WARN
#define RS_INFO_LOG   rs::LoggerLevel::INFO
#define RS_DEBUG_LOG  rs::LoggerLevel::DEBUG
#define RS_TRACE_LOG  rs::LoggerLevel::TRACE

// clang-format off
#define RS_LOGGER_WRAPPING_LIST() \
 public: \
   template <typename... Args> void fatal(RS_LOGGER_FORMAT_TYPE fmt, Args... args)  { logImpl(LoggerLevel::FATAL, RS_LOGGER_FORMAT_CAST(fmt), args...); } \
   template <typename... Args> void error(RS_LOGGER_FORMAT_TYPE fmt, Args... args)  { logImpl(LoggerLevel::ERROR, RS_LOGGER_FORMAT_CAST(fmt), args...); } \
   template <typename... Args> void warn(RS_LOGGER_FORMAT_TYPE fmt, Args... args)   { logImpl(LoggerLevel::WARN,  RS_LOGGER_FORMAT_CAST(fmt), args...); } \
   template <typename... Args> void info(RS_LOGGER_FORMAT_TYPE fmt, Args... args)   { logImpl(LoggerLevel::INFO,  RS_LOGGER_FORMAT_CAST(fmt), args...); } \
   template <typename... Args> void debug(RS_LOGGER_FORMAT_TYPE fmt, Args... args)  { logImpl(LoggerLevel::DEBUG, RS_LOGGER_FORMAT_CAST(fmt), args...); } \
   template <typename... Args> void trace(RS_LOGGER_FORMAT_TYPE fmt, Args... args)  { logImpl(LoggerLevel::TRACE, RS_LOGGER_FORMAT_CAST(fmt), args...); } \
   template <typename... Args> void log(bool result, RS_LOGGER_FORMAT_TYPE fmt, Args... args) { result ? info(fmt, args...) : error(fmt, args...); }
// clang-format on

struct LoggerProperty
{
  enum Boolean : int
  {
    eLoggerInherit = -1,
    eLoggerTrue    = static_cast<int>(true),
    eLoggerFalse   = static_cast<int>(false),
  };

  LoggerProperty() = default;
  LoggerProperty(LoggerLevel level) : level(level) {}
  LoggerProperty(const LoggerProperty& props) { *this = props; }

  // Properties
  LoggerLevel level     = LoggerLevel::INFO;
  std::string category  = RS_LOGGER_DEFAULT_CATEGORY;
  std::string directory = RS_LOGGER_DEFAULT_DIRECTORY;
  int         style     = LOGGER_STYLE_UNSET;

  // --- Attributes ---
  // Configure & Attribute (both)
  int using_header_date      = eLoggerInherit;
  int using_micro_resolution = eLoggerInherit;

  // Logging Attribute
  int raw_logging    = eLoggerInherit;
  int ignore_level   = eLoggerInherit;
  int styled_logging = eLoggerInherit;  // style이 지정된 경우 무시됨
};

class LoggerBase
{
  friend class Logger;

 protected:
  struct Context
  {
    struct Header
    {
      std::string string           = "";
      uint64_t    tick             = 0;   // 로그 발생 시간 (tick)
      char        micro_time[27]   = {};  // 로그 발생 시간 (데이터베이스 Primary Key) (yyyy-mm-dd hh:mm:ss.xxxxxx)
      char        date_only[11]    = {};  // 로그 발생 날짜 (날짜별 로그 파일 생성용) (yyyy-mm-dd)
      char        timestamp[27]    = {};  // 로그 발생 시간 (yyyy-mm-dd hh:mm:ss.xxx or xxxxxx)
      char        level_string[10] = {};  // 로그 레벨 (SILENT, FATAL, ERROR, WARN, INFO, DEBUG, TRACE)
    } header;

    struct Body
    {
      std::string string = "";
    } body;

    struct Footer
    {
      std::string string = "";
      const char* file   = nullptr;
      int         line   = 0;
    } footer;
  };

 public:
  virtual ~LoggerBase() = default;

 protected:
  LoggerBase(const LoggerProperty& props) : default_props_(props) {}
  LoggerBase()                             = default;
  LoggerBase(const LoggerBase&)            = delete;
  LoggerBase& operator=(const LoggerBase&) = delete;

 public:
  // Get Property
  LoggerProperty getProperty() const { return default_props_; }

  // Minimal Logging Level
  void        setLoggerLevel(LoggerLevel level) { minimal_level_ = level; }
  LoggerLevel getLoggerLevel() const { return minimal_level_; }

  // Category
  void        setCategory(const std::string& category) { default_props_.category = category; }
  std::string getCategory() const { return default_props_.category; }

  // Using Header Date
  void setHeaderDate(bool use) { default_props_.using_header_date = use; }
  bool getUsingHeaderDate() const { return default_props_.using_header_date; }

  // Using Micro Resolution
  void setMicroResolution(bool use) { default_props_.using_micro_resolution = use; }
  bool getUsingMicroResolution() const { return default_props_.using_micro_resolution; }

  // Color Logging
  void setColorLogging(bool use) { default_props_.styled_logging = use; }
  bool getColorLogging() const { return default_props_.styled_logging; }

 protected:
  bool assertLevel(LoggerLevel level, const LoggerProperty* props = nullptr) const;
  bool isDebugging() const { return assertLevel(LoggerLevel::DEBUG); }
  bool isVerbose() const { return assertLevel(LoggerLevel::TRACE); }

  void updateInheritProperties(LoggerProperty& props) const;

 protected:
  static void setHeaderTimestamp(const LoggerProperty& props, Context* context);
  static void setHeaderContext(LoggerLevel level, const LoggerProperty& props, Context* context);
  template <typename... Args>
  static void setBodyContext(const LoggerProperty& props, Context* context, const char* format, Args&&... args);
  static void setFooterContext(const LoggerProperty& props, Context* context);

  static std::string generateContent(LoggerLevel level, const LoggerProperty& props, Context* context, bool newline = true);

 private:
  static std::string generateEscapeCode(const int style, const bool for_string_print = false);

 protected:
  static const char*
  loggerLevelString(LoggerLevel level, bool with_brackets = false)
  {
    switch (level)
    {
      case LoggerLevel::SILENT: return with_brackets ? "[SILENT]" : "SILENT";
      case LoggerLevel::FATAL:  return with_brackets ? "[FATAL]" : "FATAL";
      case LoggerLevel::ERROR:  return with_brackets ? "[ERROR]" : "ERROR";
      case LoggerLevel::WARN:   return with_brackets ? "[WARN]" : "WARN";
      case LoggerLevel::INFO:   return with_brackets ? "[INFO]" : "INFO";
      case LoggerLevel::DEBUG:  return with_brackets ? "[DEBUG]" : "DEBUG";
      case LoggerLevel::TRACE:  return with_brackets ? "[TRACE]" : "TRACE";
      default:                  return with_brackets ? "[NULL]" : "NULL";
    }
  }

 protected:
  LoggerLevel    minimal_level_ = LoggerLevel::INFO;
  LoggerProperty default_props_ = {};
};

template <typename... Args>
inline void LoggerBase::setBodyContext(const LoggerProperty& props, Context* context, const char* format, Args&&... args)
{
  if (context == nullptr || !context->body.string.empty())
    return;

  context->body.string = rs::format(format, std::forward<Args>(args)...);
}

std::optional<LoggerLevel> toLoggerLevel(std::string level);

};  // namespace rs
