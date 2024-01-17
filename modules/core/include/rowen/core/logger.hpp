#ifndef ROWEN_SDK_CORE_LOGGER_HPP
#define ROWEN_SDK_CORE_LOGGER_HPP

#include <cstring>
#include <mutex>
#include <string>
#include <unordered_map>

#include "function.hpp"

namespace rs {

class Logger
{
  static constexpr char DEFAULT_CATEGORY[]   = "system";
  static constexpr char DEFAULT_DIRECTORY[]  = "log";
  static constexpr char PREVIOUS_DIRECTORY[] = "backup/";
  static constexpr int  MAXIMUM_PACKET_SIZE  = 8192;

 public:
  // --- Level -------------------------------------------------------------------
  enum class Level : int
  {
    SILENT,  // 모든 로그 끄기
    FATAL,   // 운영 간 치명적인 오류 및 예외 상황
    ERROR,   // 기능 동작 실패
    WARN,    // 기능 동작은 실패했으나 운영에 영향 없음 또는 기능은 성공했으나, 예상치 못한 상황
    INFO,    // 일반 로그
    DEBUG,   // 디버깅 관련 로그 (개발자에게 필요한 로그)
  };

 private:  // Can't access from outside
  static constexpr auto LAST_LEVEL   = static_cast<int>(Level::DEBUG);
  static constexpr auto LEVEL__TRACE = static_cast<Level>(LAST_LEVEL + 1);
  static constexpr auto LEVEL____RAW = static_cast<Level>(LAST_LEVEL + 2);

  static const char* LOGGER_LEVEL(Logger::Level level, bool with_brackets = false);

  // --- Target & Properties -----------------------------------------------------
 public:
  // enum Target
  static constexpr uint8_t CONSOLE  = 0b00000001;  // 콘솔(CLI) 출력
  static constexpr uint8_t FILE     = 0b00000010;  // 파일 기록

  // Logger에서 사용되는 속성 목록
  struct Property
  {
    std::string directory        = DEFAULT_DIRECTORY;
    std::string category         = DEFAULT_CATEGORY;
    Level       level            = Level::INFO;
    uint8_t     target           = CONSOLE | FILE;
    bool        trace_enable     = true;
    bool        set_header_date  = false;
    bool        micro_resolution = false;  // 소수점 6자리까지 표기
  };

  // --- Implement ---------------------------------------------------------------
 public:
  Logger(const Logger& other)            = delete;
  Logger& operator=(const Logger& other) = delete;
  ~Logger();

  /*
   * @brief Logger 생성자
   * @param prop Logger의 속성
   */
  Logger(const Property& prop);

  /*
   * @brief Logger 생성자 (Default Property 사용)
   */
  Logger();

  /*
   * @brief Logger 생성자
   * @param prop Logger의 속성
   */
  Logger(const std::string& directory, const std::string& category);

  //////////////////////////
  // Options & Control

  // clang-format off
  void setLevel(Level level)                    { props_.level = level; }
  void setTarget(uint8_t write_target)          { props_.target = write_target; }
  void setCategory(const std::string& category) { props_.category = category; }
  void setTraceLogging(bool set)                { props_.trace_enable = set; }
  void setHeaderDate(bool set)                  { props_.set_header_date = set; }
  void setMicroResolution(bool set)             { props_.micro_resolution = set; }
  
  void setDirectory(const std::string& directory);
  void resetDirectory();

  void setProperties(const Property& prop);

  std::string category() const  { return props_.category; }
  std::string directory() const { return props_.directory; }

  // clang-format on

  //////////////////////////
 private:
  // 출력할 로그 정보
  struct Content
  {
    // Common
    Level       level;  // 로그 레벨
    bool        raw;    // raw 로그 여부
    const char* file;   // 로그 발생 위치
    int         line;   // 로그 발생 위치

    // Contents
    uint64_t    tick           = 0;   // 로그 발생 시간 (tick)
    char        timestamp[27]  = {};  // 로그 발생 시간 (yyyy-mm-dd hh:mm:ss.xxx or xxxxxx)
    char        micro_time[27] = {};  // 로그 발생 시간 (데이터베이스 Primary Key) (yyyy-mm-dd hh:mm:ss.xxxxxx)
    char        date_only[11]  = {};  // 로그 발생 날짜 (날짜별 로그 파일 생성용) (yyyy-mm-dd)
    std::string body;                 // 로그 내용
  };

 public:
  // clang-format off
  // wrapper
  template <typename... Args> void fatal(const char* fmt, Args... args)  { log(Level::FATAL, false, nullptr, 0, fmt, args...); }
  template <typename... Args> void error(const char* fmt, Args... args)  { log(Level::ERROR, false, nullptr, 0, fmt, args...); }
  template <typename... Args> void warn(const char* fmt, Args... args)   { log(Level::WARN,  false, nullptr, 0, fmt, args...); }
  template <typename... Args> void info(const char* fmt, Args... args)   { log(Level::INFO,  false, nullptr, 0, fmt, args...); }
  template <typename... Args> void debug(const char* fmt, Args... args)  { log(Level::DEBUG, false, nullptr, 0, fmt, args...); }
  template <typename... Args> void trace(const char* fmt, Args... args)  { log(LEVEL__TRACE, false, nullptr, 0, fmt, args...); }

  // wrapper (no time-line & file location)
  template <typename... Args> void fatal_raw(const char* fmt, Args... args)  { log(Level::FATAL, true, nullptr, 0, fmt, args...); }
  template <typename... Args> void error_raw(const char* fmt, Args... args)  { log(Level::ERROR, true, nullptr, 0, fmt, args...); }
  template <typename... Args> void warn_raw(const char* fmt, Args... args)   { log(Level::WARN,  true, nullptr, 0, fmt, args...); }
  template <typename... Args> void info_raw(const char* fmt, Args... args)   { log(Level::INFO,  true, nullptr, 0, fmt, args...); }
  template <typename... Args> void debug_raw(const char* fmt, Args... args)  { log(Level::DEBUG, true, nullptr, 0, fmt, args...); }
  template <typename... Args> void trace_raw(const char* fmt, Args... args)  { log(LEVEL__TRACE, true, nullptr, 0, fmt, args...); }
  // clang-format on

 public:
  void log(Level level, bool raw, const char* file, int line, const char* fmt, ...);

 private:
  void writeConsole(const Content& content);
  void writeFile(const Content& content);

 private:
  Property props_;

  // Logger에서 setDirectory된 폴더 목록들 (데이터 삭제용)
  struct FileProperty
  {
    std::mutex locker;          // 파일별 접근 제어
    bool       startup = true;  // 파일별 최초 실행 여부 (디렉토리 + 카테고리 백업 생성 여부)
  };

  struct FilePropertyKey
  {
    std::string directory;
    std::string category;

    bool operator==(const FilePropertyKey& other) const
    {
      return (directory == other.directory) && (category == other.category);
    }

    struct Hash
    {
      std::size_t operator()(const FilePropertyKey& k) const
      {
        return ((std::hash<std::string>()(k.directory) ^ (std::hash<std::string>()(k.category) << 1)) >> 1);
      }
    };
  };

  using FileProperties = std::unordered_map<FilePropertyKey, FileProperty, FilePropertyKey::Hash>;
  static FileProperties file_properties_;
  static std::mutex     file_properties_locker_;
  FileProperty*         file_property_ = nullptr;

 public:
  static bool removeStaleFiles(int deprecated_days);
};

};  // namespace rs

inline rs::Logger  g_logger;
inline rs::Logger& logger = g_logger;

// clang-format off
#define logger_fatal(fmt, ...) (logger.log(rs::Logger::Level::FATAL, false, __FILENAME__, __LINE__, (const char*)fmt, ##__VA_ARGS__))
#define logger_error(fmt, ...) (logger.log(rs::Logger::Level::ERROR, false, __FILENAME__, __LINE__, (const char*)fmt, ##__VA_ARGS__))
#define logger_warn(fmt, ...)  (logger.log(rs::Logger::Level::WARN,  false, __FILENAME__, __LINE__, (const char*)fmt, ##__VA_ARGS__))
#define logger_info(fmt, ...)  (logger.log(rs::Logger::Level::INFO,  false, __FILENAME__, __LINE__, (const char*)fmt, ##__VA_ARGS__))
#define logger_debug(fmt, ...) (logger.log(rs::Logger::Level::DEBUG, false, __FILENAME__, __LINE__, (const char*)fmt, ##__VA_ARGS__))
#define logger_trace(fmt, ...) (logger.log(rs::Logger::Level::TRACE, false, __FILENAME__, __LINE__, (const char*)fmt, ##__VA_ARGS__))
#define logger_errno(_TITLE_, _STR_)	{ logger.error("%s : %s (%d, %s)", _TITLE_, _STR_, errno, strerror(errno)); }
// clang-format on

#endif  // ROWEN_SDK_CORE_LOGGER_HPP
