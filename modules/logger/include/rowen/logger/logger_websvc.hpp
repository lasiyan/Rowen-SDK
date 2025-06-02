#ifdef RS_WITH_DATABASE_LOGGER
  #pragma once

  #include <atomic>
  #include <condition_variable>
  #include <mutex>
  #include <queue>
  #include <rowen/logger/detail/logger_base.cpp>
  #include <thread>

namespace rs {

class WebsvcLogger : public LoggerBase
{
  friend class Logger;

  struct Context
  {
    uint64_t    tick;        // 로그 발생 시간 (context->header.tick)
    std::string micro_time;  // 로그 발생 시간 (context->header.micro_time)
    std::string level;       // 로그 레벨 (loggerLevelString(level))
    std::string content;     // 로그 내용 (context->body.string)
  };
  using ContextT = std::unique_ptr<Context>;

 public:
  WebsvcLogger();
  ~WebsvcLogger();

  void        setDatabaseHost(const std::string& host);
  void        setDatabasePort(const int port);
  void        setDatabaseUser(const std::string& user);
  void        setDatabasePassword(const std::string& password);
  const char* getDatabaseHost() const;
  int         getDatabasePort() const;
  const char* getDatabaseUser() const;
  const char* getDatabasePassword() const;

 private:
  void threadFuncWriteToWEBSVC();

 private:
  std::thread             logger_thread_;
  std::atomic_bool        logger_thread_stop_;
  std::mutex              content_locker_;
  std::queue<ContextT>    content_queue_;
  std::condition_variable content_notify_;

  // Database connection
  class Database;
  std::unique_ptr<Database> database;

  /* --------------------------------------------------------------------------- */
  /**
   * @brief 사용: logger.websvc.log(props, "Hello World");
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
    logImpl(props.level, props, &context, RS_LOGGER_FORMAT_CAST(fmt), args...);
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
   * @brief 사용: logger.database.[LOG_LEVEL]("Hello World");
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
  #ifdef __unsupport_mariadb__
    return;
  #endif

    if (assertLevel(level, &props) == false)
      return;

    if (context == nullptr)
      return;

    // Fill context (if not exist ..)
    LoggerBase::setHeaderContext(level, props, context);
    LoggerBase::setBodyContext(props, context, fmt, args...);
    LoggerBase::setFooterContext(props, context);

    // Write to WEBSVC
    std::unique_lock locker(content_locker_);
    auto             context_ptr = std::make_unique<Context>();
    context_ptr->tick            = context->header.tick;
    context_ptr->micro_time      = context->header.micro_time;
    context_ptr->level           = loggerLevelString(level);
    context_ptr->content         = context->body.string;
    content_queue_.push(std::move(context_ptr));
    content_notify_.notify_one();
  }
};

};  // namespace rs

#endif  // RS_WITH_DATABASE_LOGGER
