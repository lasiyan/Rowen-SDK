#pragma once

#include <cassert>
#include <filesystem>
#include <mutex>
#include <rowen/core/function.hpp>  // IWYU pragma: export
#include <rowen/logger/detail/logger_base.hpp>
#include <unordered_map>

namespace rs {

class FileLogger : public LoggerBase
{
  friend class Logger;

  static constexpr size_t RS_LOGGER_MAXIMUM_PROPERTY_COUNT = 128;
  static constexpr float  RS_LOGGER_MAXIMUM_PROPERTY_RATIO = 0.75f;
  static constexpr size_t RS_LOGGER_BUCKET_SIZE            = RS_LOGGER_MAXIMUM_PROPERTY_COUNT / RS_LOGGER_MAXIMUM_PROPERTY_RATIO;

 public:
  static bool removeStaleFiles(int deprecated_days);

 public:
  FileLogger() : LoggerBase()
  {
    default_props_.using_header_date      = false;
    default_props_.using_micro_resolution = false;
    default_props_.raw_logging            = false;
    default_props_.ignore_level           = false;
    default_props_.styled_logging         = true;
  }
  ~FileLogger() = default;

  // file logging directory
  void        setDirectory(const std::string& directory) { default_props_.directory = directory; }
  std::string getDirectory() const { return default_props_.directory; }
  void        resetDirectory() { default_props_.directory = RS_LOGGER_DEFAULT_DIRECTORY; }

 private:
  // Logger에서 setDirectory된 폴더 목록들 (데이터 삭제용)
  struct FileProperty
  {
    mutable std::mutex locker;          // 파일별 접근 제어
    mutable bool       startup = true;  // 파일별 최초 실행 여부 (디렉토리 + 카테고리 백업 생성 여부)
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
#if 1
        std::size_t hash1    = std::hash<std::string>()(k.directory);
        std::size_t hash2    = std::hash<std::string>()(k.category);
        std::size_t combined = hash1 ^ (hash2 + 0x9e3779b9 + (hash1 << 6) + (hash1 >> 2));
        return combined;
#else
        return ((std::hash<std::string>()(k.directory) ^ (std::hash<std::string>()(k.category) << 1)) >> 1);
#endif
      }
    };
  };

  struct FilePropertyPair
  {
    const FilePropertyKey* key   = nullptr;
    const FileProperty*    value = nullptr;
  };

  using FileProperties = std::unordered_map<FilePropertyKey, FileProperty, FilePropertyKey::Hash>;
  static FileProperties file_properties_;
  static std::mutex     file_properties_locker_;

 private:
  void             makeSurePath(std::string& target_dir, std::string& target_category);
  FilePropertyPair selectFileProperty(std::string target_dir, std::string target_category);
  FilePropertyPair insertFileProperty(std::string target_dir, std::string target_category);

  /* --------------------------------------------------------------------------- */
  /**
   * @brief 사용: logger.file.log(props, "Hello World");
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
   * @brief 사용: logger.file.[LOG_LEVEL]("Hello World");
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

    // ------ select file property ------
    auto property_set = selectFileProperty(props.directory, props.category);
    assert((property_set.value != nullptr) && "file_property is nullptr");

    if (property_set.key == nullptr || property_set.value == nullptr)
      return;

    // ------ Fill context (if not exist ..) ------
    LoggerBase::setHeaderContext(level, props, context);
    LoggerBase::setBodyContext(props, context, fmt, std::forward<Args>(args)...);
    LoggerBase::setFooterContext(props, context);

    // Generate Content
    auto content = LoggerBase::generateContent(level, props, context, true);

    auto filestem = rs::format("%s-%s", context->header.date_only, property_set.key->category.c_str());
    auto filepath = rs::format("%s%s.txt", property_set.key->directory.c_str(), filestem.c_str());

    std::scoped_lock locker(property_set.value->locker);

    // 만약 프로그램 시작 시점(첫 로깅)에서
    if (property_set.value->startup)
    {
      property_set.value->startup = false;

      // 동일한 이름의 파일이 존재한다면(프로그램 비정상 종료로 인한)
      if (std::filesystem::exists(filepath))
      {
        // 1. 폴더 안에 filepath와 동일한 이름이 포함된 파일이 총 몇개인지 찾는다.
        int  total      = 0;
        auto backup_dir = rs::format("%s%s/", property_set.key->directory.c_str(), RS_LOGGER_BACKUP_DIRECTORY);

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
          std::filesystem::create_directories(backup_dir);

        // 3. 기존 파일을 백업 폴더로 이동시키고 이름을 변경한다
        auto backup_file = rs::format("%s%s_%05d.txt", backup_dir.c_str(), filestem.c_str(), total + 1);
        std::filesystem::rename(filepath, backup_file);
      }
    }

    if (auto file = std::fopen(filepath.c_str(), "ab+"); file)
    {
      fprintf(file, "%s", content.c_str());
      fclose(file);
    }
  }
};

};  // namespace rs
