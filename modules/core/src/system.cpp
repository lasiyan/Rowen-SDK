#include <cstring>
#include <regex>
#include <rowen/core/function.hpp>
#include <rowen/core/time.hpp>

namespace rs {

std::string __platform_terminal_implement(bool sudo, const std::string& raw_cmd)
{
  std::string cmd = raw_cmd;
  if (sudo)
    cmd = format("echo '1' | sudo -S -p \"\" %s", cmd.c_str());
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

rs::response_t kill_processor(const char* process_name, const float terminate_timeout)
{
  rs::response_t response;

  constexpr bool as_sudo = true;

  // 1. check if process is running by pgrep
  auto pgrep_result = rs::terminal(false, "pgrep %s", process_name);

  if (pgrep_result.empty())
    return response.set(rssTerminated, "No %s process is running.", process_name);

  // 2. terminate process by pkill
  rs::terminal(true, "pkill -9 %s", process_name);

  // 3. wait for process to terminate
  if (terminate_timeout > 0.0f)
  {
    auto start_time = rs::time::tick<milliseconds>();

    while (true)
    {
      rs::time::sleep(500ms);

      pgrep_result = rs::terminal(false, "pgrep %s", process_name);

      if (pgrep_result.empty())
        break;

      if (rs::time::elapsed(terminate_timeout, start_time) == true)
      {
        response.set(rssInUseResource, "timeout to terminate %s processes.", process_name);
        break;
      }
      else
      {
        // re-try to terminate process
        rs::terminal(true, "pkill -9 %s", process_name);
      }
    }
  }

  // 4. check if process is terminated
  pgrep_result = rs::terminal(false, "pgrep %s", process_name);

  if (pgrep_result.empty())
  {
    response.set(rssTerminated, "All %s processes are terminated.", process_name);
    return true;
  }
  else
  {
    response.set(rssInUseResource, "failed to terminate %s processes.", process_name);
    return false;
  }
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
