#ifdef RS_WITH_DATABASE_LOGGER

  #include <rowen/logger/logger_websvc.hpp>
  #include <rowen_3rd/jsoncpp.hpp>
  #include <rowen_3rd/websvc_conn.hpp>

namespace rs {

class WebsvcLogger::Database
{
  friend class WebsvcLogger;

 public:
  websvc::Connector connector;
};

WebsvcLogger::WebsvcLogger() : LoggerBase()
{
  default_props_.using_header_date      = false;
  default_props_.using_micro_resolution = false;
  default_props_.raw_logging            = false;
  default_props_.ignore_level           = false;
  default_props_.styled_logging         = false;

  database = std::make_unique<Database>();
  database->connector.ensureConnection();

  #ifndef __unsupport_mariadb__
  logger_thread_ = std::thread(&WebsvcLogger::threadFuncWriteToWEBSVC, this);
  #endif
}

WebsvcLogger::~WebsvcLogger()
{
  std::unique_lock locker(content_locker_);
  logger_thread_stop_ = true;
  content_notify_.notify_all();
  locker.unlock();

  #ifndef __unsupport_mariadb__
  if (logger_thread_.joinable())
    logger_thread_.join();
  #endif
}

void WebsvcLogger::threadFuncWriteToWEBSVC()
{
  #ifdef _DEBUG
  printf("WebsvcLogger::threadFunc startup\n");
  #endif

  logger_thread_stop_ = false;
  while (true /* logger_thread_stop_ == false */)
  {
    std::unique_lock locker(content_locker_);
    content_notify_.wait(locker, [this] { return !content_queue_.empty() || logger_thread_stop_; });

    if (logger_thread_stop_)
    {
      // Queue에 데이터가 남아있고, 마지막 데이터 처리가 Fail 상태가 아닌 경우, DB가 연결되어 있으므로 잔여 데이터 처리
      if (content_queue_.empty())
        break;

      if (database->connector.error().empty() == false)
        break;

  #ifdef _DEBUG
      printf("WebsvcLogger::threadFunc remaining data processing ... (%lu ea)\n", content_queue_.size());
  #endif
    }

    if (content_queue_.empty())
      continue;

    auto context = std::move(content_queue_.front());
    content_queue_.pop();
    locker.unlock();

    // 데이터베이스 연결
    database->connector.ensureConnection();

    // 쿼리 생성
    std::string query = "INSERT INTO TB_LOGS_MAIN("
                        "LOGS_TICK, "
                        "LOGS_DT, "
                        "LOGS_TYPE, "
                        "LOGS_DESC) "
                        "VALUES (?, ?, ?, ?)";

    // 쿼리 실행
    auto res = database->connector.executeUpdate(query,
                                                 context->tick,
                                                 context->micro_time,
                                                 context->level,
                                                 context->content);

    // 연결이 되어 있는데, insert에 실패한 경우만 로깅
    if (res == false && database->connector.isConnected())
      fprintf(stderr, "WebsvcLogger::threadFunc : Failed to execute query : %s\n", database->connector.cerror());
  }

  // Release Resources
  database->connector.disconnect();

  // Pop remaining data
  while (!content_queue_.empty())
    content_queue_.pop();

  #ifdef _DEBUG
  printf("WebsvcLogger::threadFunc terminated\n");
  #endif
}

// -------------------------------------------------------------------------------

// clang-format off
void WebsvcLogger::setDatabaseHost(const std::string& host)         { database->connector.setProperties("host", host); }
void WebsvcLogger::setDatabasePort(const int port)                  { database->connector.setProperties("port", std::to_string(port)); }
void WebsvcLogger::setDatabaseUser(const std::string& user)         { database->connector.setProperties("user", user); }
void WebsvcLogger::setDatabasePassword(const std::string& password) { database->connector.setProperties("password", password); }

const char* WebsvcLogger::getDatabaseHost() const                   { return database->connector.getConnectionProperties().host.c_str(); }
int         WebsvcLogger::getDatabasePort() const                   { return database->connector.getConnectionProperties().port; }
const char* WebsvcLogger::getDatabaseUser() const                   { return database->connector.getConnectionProperties().user.c_str(); }
const char* WebsvcLogger::getDatabasePassword() const               { return database->connector.getConnectionProperties().password.c_str(); }
// clang-format on

}  // namespace rs

#endif  // RS_WITH_DATABASE_LOGGER
