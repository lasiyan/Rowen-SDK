#if (__cplusplus < __cpp_lib_filesystem)
  #include "rowen/utils/file.hpp"

  #include <dirent.h>
  #include <fcntl.h>
  #include <sys/stat.h>
  #include <unistd.h>

  #include <cstdarg>
  #include <fstream>
  #include <iostream>

namespace rs {
namespace utils {
namespace file {

bool exist(const std::string& title)
{
  struct stat buffer;
  return (stat(title.c_str(), &buffer) == 0);
}

int size(const std::string& title)
{
  int   size = 0;
  FILE* fp;
  if ((fp = fopen(title.c_str(), "rb")) != nullptr)
  {
    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    fclose(fp);
  }
  return size;
}

bool data(const std::string& title, uint8_t* buffer, int size)
{
  if (size == 0)
    size = file::size(title);

  if (auto fp = fopen(title.c_str(), "rb"); fp)
  {
    auto read_size = fread(buffer, 1, size, fp);
    fclose(fp);
    return (read_size > 0);
  }

  return false;
}

void create(const std::string& file, mode_t mode)
{
  close(creat(file.c_str(), mode));
}

int remove(const std::string& file)
{
  char command[512];
  sprintf(command, "rm -f %s", file.c_str());
  return system((const char*)command);
}

void write(const std::string& file, const char* fmt, ...)
{
  std::fstream output_stream;
  output_stream.open(file.c_str(), std::ios::out | std::ios::app);

  va_list args;
  char    buffer[1024 * 1024];
  va_start(args, fmt);
  vsnprintf(buffer, sizeof(buffer), fmt, args);
  va_end(args);

  output_stream << buffer << std::endl;
  output_stream.close();
}

void createDir(const std::string& path, mode_t mode, bool delete_if_exist)
{
  struct stat status    = { 0 };
  auto        mkdir_cmd = "mkdir -p %s -m %d";

  if (stat(path.c_str(), &status) == -1)
  {  // No Exist Dir
    char command[512];
    snprintf(command, sizeof(command), mkdir_cmd, path.c_str(), mode);
    system((const char*)command);
  }
  else
  {
    if (delete_if_exist)
    {
      char command[512];  // remove exist dir
      snprintf(command, sizeof(command), "rm -rf %s", path.c_str());
      system((const char*)command);

      snprintf(command, sizeof(command), mkdir_cmd, path.c_str(), mode);
      system((const char*)command);
    }
  }
}

std::vector<std::string> list(const std::string& path, bool only_file)
{
  std::vector<std::string> output;

  DIR*          dir = nullptr;
  class dirent* ent = nullptr;
  class stat    st;

  dir = opendir(path.c_str());
  while (dir && (ent = readdir(dir)))
  {
    const std::string file_name      = ent->d_name;
    const std::string full_file_name = path + "/" + file_name;

    if (file_name[0] == '.')
      continue;

    if (stat(full_file_name.c_str(), &st) == -1)
      continue;

    const bool is_directory = (st.st_mode & S_IFDIR) != 0;
    if (only_file && is_directory)
      continue;

    output.push_back(file_name);
  }
  closedir(dir);

  return output;
}

}  // namespace file
}  // namespace utils
}  // namespace rs

#endif
