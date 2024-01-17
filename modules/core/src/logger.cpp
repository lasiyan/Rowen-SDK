#include "rowen/core/logger.hpp"

#include <cassert>
#include <cstdarg>
#include <filesystem>
#include <vector>

#include "rowen/core/function.hpp"

#ifdef _WIN32
  #include <Windows.h>
  #include <direct.h>
  #include <io.h>
  #include <sys/timeb.h>
  #undef ERROR
#endif

rs::Logger::FileProperties rs::Logger::file_properties_;
std::mutex                 rs::Logger::file_properties_locker_;

namespace rs {

const char* Logger::LOGGER_LEVEL(Logger::Level level, bool with_brackets)
{
  switch (level)
  {
    case Logger::Level::SILENT: return with_brackets ? "[SILENT]" : "SILENT";
    case Logger::Level::FATAL:  return with_brackets ? "[FATAL]" : "FATAL";
    case Logger::Level::ERROR:  return with_brackets ? "[ERROR]" : "ERROR";
    case Logger::Level::WARN:   return with_brackets ? "[WARN]" : "WARN";
    case Logger::Level::INFO:   return with_brackets ? "[INFO]" : "INFO";
    case Logger::Level::DEBUG:  return with_brackets ? "[DEBUG]" : "DEBUG";
    case Logger::LEVEL__TRACE:  return with_brackets ? "[TRACE]" : "TRACE";
    case Logger::LEVEL____RAW:  return with_brackets ? "[RAW]" : "RAW";
    default:                    return with_brackets ? "[NULL]" : "NULL";
  }
}

// -----------------------------------------------------------------------------

Logger::Logger(const Property& prop)
{
  props_ = prop;
}

Logger::Logger() : Logger(Property{})
{
  // with default prop
}

Logger::Logger(const std::string& directory, const std::string& category)
    : Logger{ Property{ .directory = directory, .category = category } }
{
  // with default prop
}

Logger::~Logger()
{
}

// -- Options & Control ----------------------------------------------------------
void Logger::setDirectory(const std::string& dir)
{
  std::unique_lock<std::mutex> ulock(file_properties_locker_);

  // Make sure path
  const auto& execute_path = get_execute_path();

  if (dir.find(execute_path) == std::string::npos)
  {
    props_.directory = rs::format("%s%s", execute_path.c_str(), dir.c_str());
  }
  else
  {
    props_.directory = dir;
  }

  // Assert directory
  if (props_.directory.back() != '/')
    props_.directory.push_back('/');

  // if directory is not exist path, create (recursive)
  std::string directory_with_backup = props_.directory + PREVIOUS_DIRECTORY;
  if (std::filesystem::exists(directory_with_backup) == false)
  {
    // 백업 디렉토리 포함 생성
    std::filesystem::create_directories(directory_with_backup);
    rs::terminal(true, "chmod 777 -R %s", props_.directory.c_str());
  }

  // -- Set directory ------------------------------------------------------------
  struct FilePropertyKey key_value = {
    .directory = props_.directory,
    .category  = props_.category,
  };

  auto iter = file_properties_.find(key_value);
  if (iter == file_properties_.end())
  {
    file_properties_.emplace(std::piecewise_construct,
                             std::forward_as_tuple(key_value),
                             std::forward_as_tuple());

    auto bucket = file_properties_.bucket(key_value);
    if (file_properties_.bucket_size(bucket) > 1)
    {
      assert("********* Hash collision detected *********");
    }
    else
    {
      file_property_ = &file_properties_.at(key_value);
    }
  }
  else
  {
    file_property_ = &iter->second;
  }
}

void Logger::resetDirectory()
{
  setDirectory(DEFAULT_DIRECTORY);
}

void Logger::setProperties(const Property& prop)
{
  setCategory(prop.category);
  setLevel(prop.level);
  setTarget(prop.target);
  setTraceLogging(prop.trace_enable);
  setHeaderDate(prop.set_header_date);
  setMicroResolution(prop.micro_resolution);
  setDirectory(prop.directory);
}

//---------------------------------------------------------------
// implement
void Logger::log(Level level, bool raw, const char* file, int line, const char* format, ...)
{
  if (file_property_ == nullptr)
  {
    setDirectory(props_.directory);
    assert((file_property_ != nullptr) && "file_property_ is nullptr");
  }

  if (file_property_ == nullptr)
    return;

  std::unique_lock<std::mutex> ulock(file_property_->locker);

  try
  {
    Content content{
      .level = level,
      .raw   = raw,
      .file  = file,
      .line  = line,
    };

    //---------------------------------------------------------------
    // Logger Level 및 Write Target 설정
    bool write_console = false, write_file = false;

    switch (level)
    {
      case Level::FATAL:
      case Level::ERROR:
      case Level::WARN:
      case Level::INFO:
      case Level::DEBUG:
        if (level <= props_.level)
        {
          write_console = props_.target & Logger::CONSOLE;
          write_file    = props_.target & Logger::FILE;
        }
        break;
      case LEVEL__TRACE:
        if (props_.trace_enable)
        {
          write_file = props_.target & Logger::FILE;
        }
        break;
      default:
        return;
    };

    if (write_console == false && write_file == false)
      return;

    //---------------------------------------------------------------
    // Timestamp

    struct timespec ts;
    struct tm       tm_info;

    clock_gettime(CLOCK_REALTIME, &ts);

    time_t sec = ts.tv_sec;
    localtime_r(&sec, &tm_info);
    content.tick = sec * 1000000000 + ts.tv_nsec;

    // micro time (19: YYYY-mm-DD HH:MM:SS, 8: .xxxxxx'\0')
    strftime(content.micro_time, sizeof(content.micro_time), "%Y-%m-%d %H:%M:%S", &tm_info);
    snprintf(content.micro_time + 19, 8, ".%06ld", ts.tv_nsec / 1000);

    // date only (10: YYYY-mm-DD)
    memcpy(content.date_only, content.micro_time, 10);

    // timestemp (10: YYYY-mm-DD)
    int tmpos = 0;
    if (props_.set_header_date)
    {
      enum
      {
        date_only_size = 10
      };
      memcpy(content.timestamp, content.date_only, date_only_size);
      tmpos += date_only_size;
      content.timestamp[tmpos++] = ' ';  // space
    }

    // 12 or 15 : HH:MM:SS.xxx or xxxxxx
    if (props_.micro_resolution)
    {
      memcpy(content.timestamp + tmpos, content.micro_time + 11, 15);
    }
    else
    {
      memcpy(content.timestamp + tmpos, content.micro_time + 11, 12);  // drop last 3 digits
    }

    //---------------------------------------------------------------
    // Body
    va_list va_args;
    va_start(va_args, format);
    va_list va_args_copy;
    va_copy(va_args_copy, va_args);
    const int result_length = std::vsnprintf(NULL, 0, format, va_args_copy);
    va_end(va_args_copy);
    std::vector<char> result(result_length + 1);
    std::vsnprintf(result.data(), result.size(), format, va_args);
    va_end(va_args);

    content.body.assign(result.data(), result_length);

    //---------------------------------------------------------------
    // Write Log
    if (write_console)
      writeConsole(content);

    if (write_file)
      writeFile(content);
  }
  catch (std::exception& e)
  {
    fprintf(stderr, "exception : logger : %s\n", e.what());
  }
  catch (...)
  {
    fprintf(stderr, "exception : logger : undefined\n");
  }
}

void Logger::writeConsole(const Content& content)
{
  if (content.raw)
  {
    printf("%s\n", content.body.c_str());
  }
  else
  {
    printf("[%s] %-7s %s\n",
           content.timestamp, LOGGER_LEVEL(content.level, true), content.body.c_str());
  }
}

void Logger::writeFile(const Content& content)
{
  std::string footer;
  if (content.file && content.line > 0 && content.raw == false)
    footer = rs::format("  ... (%s: %d)", content.file, content.line);

  std::string str;
  if (content.raw)
  {
    str = rs::format("%s\n", content.body.c_str());
  }
  else
  {
    str = rs::format("[%s] %-7s %s%s\n", content.timestamp,
                     LOGGER_LEVEL(content.level, true), content.body.c_str(), footer.c_str());
  }

  auto filestem = rs::format("%s-%s", content.date_only, props_.category.c_str());
  auto filepath = rs::format("%s%s.txt", props_.directory.c_str(), filestem.c_str());

  // 만약 프로그램 시작 시점(첫 로깅)에서
  if (file_property_->startup)
  {
    file_property_->startup = false;

    // 동일한 이름의 파일이 존재한다면(프로그램 비정상 종료로 인한)
    if (std::filesystem::exists(filepath))
    {
      // 1. 폴더 안에 filepath와 동일한 이름이 포함된 파일이 총 몇개인지 찾는다.
      int  total      = 0;
      auto backup_dir = rs::format("%s%s", props_.directory.c_str(), PREVIOUS_DIRECTORY);

      for (const auto& entry : std::filesystem::directory_iterator(backup_dir))
      {
        if (std::filesystem::is_regular_file(entry.path()) == false)
          continue;

        if (entry.path().stem().string().find(filestem) != std::string::npos)
        {
          total++;
        }
      }

      // 2. 백업 폴더가 없다면 생성한다
      if (std::filesystem::exists(backup_dir) == false)
      {
        std::filesystem::create_directories(backup_dir);
        rs::terminal(true, "chmod 777 -R %s", props_.directory.c_str());
      }

      // 3. 기존 파일을 백업 폴더로 이동시키고 이름을 변경한다
      auto backup_file = rs::format("%s%s_%05d.txt", backup_dir.c_str(), filestem.c_str(), total + 1);
      std::filesystem::rename(filepath, backup_file);
    }
  }

  if (auto file = std::fopen(filepath.c_str(), "a"); file)
  {
    fprintf(file, "%s", str.c_str());
    fclose(file);
  }
}

// -- static methods  ------------------------------------------------------------
bool Logger::removeStaleFiles(int deprecated_days)
{
  std::unique_lock<std::mutex> ulock(file_properties_locker_);

  for (auto& [key, props] : file_properties_)
  {
    std::unique_lock<std::mutex> ulock_dir(props.locker);

    auto path = key.directory.c_str();

    rs::terminal(false, "find %s/ -name \'*.txt\' -ctime +%d -delete", path, deprecated_days);
    rs::terminal(false, "find %s/ -name \'*.txt\' -mtime +%d -delete", path, deprecated_days);
    rs::terminal(false, "find %s/ -mindepth 1 -empty -type d -delete", path);
  }
  return true;
}

}  // namespace rs

#ifdef _WIN32
  #define ERROR 0
#endif
