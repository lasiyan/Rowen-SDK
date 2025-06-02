#include <rowen/logger/logger_file.hpp>

rs::FileLogger::FileProperties rs::FileLogger::file_properties_;
std::mutex                     rs::FileLogger::file_properties_locker_;

namespace rs {

bool FileLogger::removeStaleFiles(int deprecated_days)
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

void FileLogger::makeSurePath(std::string& target_dir, std::string& target_category)
{
  const auto& execute_path = get_execute_directory();

  if (target_dir.find(execute_path) == std::string::npos)
    target_dir = execute_path / target_dir;

  if (target_dir.back() != '/')
    target_dir.push_back('/');
}

FileLogger::FilePropertyPair FileLogger::selectFileProperty(std::string target_dir, std::string target_category)
{
  makeSurePath(target_dir, target_category);

  // select file property
  {
    struct FilePropertyKey key_value = {
      .directory = target_dir,
      .category  = target_category,
    };

    std::unique_lock<std::mutex> ulock(file_properties_locker_);

    auto iter = file_properties_.find(key_value);
    if (iter == file_properties_.end())
    {
      ulock.unlock();
      return insertFileProperty(target_dir, target_category);
    }
    else
      return { &iter->first, &iter->second };
  }
}

FileLogger::FilePropertyPair FileLogger::insertFileProperty(std::string target_dir, std::string target_category)
{
  makeSurePath(target_dir, target_category);

  // select file property
  {
    struct FilePropertyKey key_value = {
      .directory = target_dir,
      .category  = target_category,
    };

    std::unique_lock<std::mutex> ulock(file_properties_locker_);

    if (file_properties_.empty())
      file_properties_.reserve(RS_LOGGER_BUCKET_SIZE);

    auto iter = file_properties_.find(key_value);
    if (iter == file_properties_.end())
    {
      file_properties_.emplace(std::piecewise_construct,
                               std::forward_as_tuple(key_value),
                               std::forward_as_tuple());

      auto bucket = file_properties_.bucket(key_value);
      if (file_properties_.bucket_size(bucket) > 1)
      {
        assert(false && "********* Hash collision detected *********");
        return {};
      }
      else
      {
        // Create newer directory
        std::string directory_with_backup = target_dir + RS_LOGGER_BACKUP_DIRECTORY + "/";
        if (std::filesystem::exists(directory_with_backup) == false)
          std::filesystem::create_directories(directory_with_backup);

        auto new_iter = file_properties_.find(key_value);
        return { &new_iter->first, &new_iter->second };
      }
    }
    else
    {
      return { &iter->first, &iter->second };
    }
  }
}

};  // namespace rs
