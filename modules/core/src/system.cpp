#include <cstring>
#include <regex>

#include "rowen/core/function.hpp"

namespace rs {

std::string __platform_terminal_implement(bool sudo, const std::string& raw_cmd)
{
  std::string cmd = raw_cmd;
  if (sudo)
    cmd = format("echo '1' | sudo -S %s", cmd.c_str());
  // printf("%s\n", cmd.c_str());
  std::string result;
  char        buffer[1024];
  ::memset(buffer, 0, sizeof(buffer));
  cmd.append(" 2>&1");
  FILE* stream = ::popen(cmd.c_str(), "r");
  if (stream)
  {
    while (::fgets(buffer, sizeof(buffer), stream) != NULL) result.append(buffer);
    ::pclose(stream);
  }

  if (sudo)  // remove sudo echo
  {
    // ex. "[sudo] password for user: " => remove
    std::regex prompt_pattern(R"(\[sudo\] password for [^:]+: )");
    result = std::regex_replace(result, prompt_pattern, "");
  }

  return result;
}

// system path
std::filesystem::path get_current_path()
{
  char buffer[1024];
  ::memset(buffer, 0, sizeof(buffer));
  ::getcwd(buffer, sizeof(buffer));
  return buffer;
}

std::filesystem::path get_execute_filepath()
{
  char buffer[1024];
  ::memset(buffer, 0, sizeof(buffer));
  auto res = ::readlink("/proc/self/exe", buffer, sizeof(buffer));
  return buffer;
}

std::filesystem::path get_execute_directory()
{
  return get_execute_filepath().parent_path();
}

std::string get_execute_directory_string()
{
  return get_execute_directory().string();
}

std::string get_execute_filename()
{
  return get_execute_filepath().filename().string();
}

}  // namespace rs
