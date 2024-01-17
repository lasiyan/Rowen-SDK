#include <cstring>

#include "rowen/core/function.hpp"

namespace rs {

std::string __rowen_terminal_implement(bool sudo, const std::string& raw_cmd)
{
  std::string cmd = raw_cmd;
  if (sudo)
    cmd = format("echo '1' | sudo -S %s", cmd.c_str());
  // printf("%s\n", cmd.c_str());
  std::string result;
  char        buffer[1024];
  memset(buffer, 0, sizeof(buffer));
  cmd.append(" 2>&1");
  FILE* stream = popen(cmd.c_str(), "r");
  if (stream)
  {
    while (fgets(buffer, sizeof(buffer), stream) != NULL) result.append(buffer);
    pclose(stream);
  }
  return result;
}

// system path
std::string get_current_path()
{
  char buffer[1024];
  memset(buffer, 0, sizeof(buffer));
  getcwd(buffer, sizeof(buffer));
  return std::string(buffer);
}

std::string get_execute_path()
{
  char buffer[1024];
  memset(buffer, 0, sizeof(buffer));
  auto res = readlink("/proc/self/exe", buffer, sizeof(buffer));
  auto str = std::string(buffer);
  if (size_t pos = str.rfind('/'); pos != std::string::npos)
  {
    str.erase(pos + 1);
  }
  return str;
}

}  // namespace rs
