#include <rowen/core/format.hpp>
#include <rowen/core/function.hpp>
#include <sstream>
#include <vector>

namespace rs {

std::string& ltrim(std::string& s, const char* t)
{
  s.erase(0, s.find_first_not_of(t));
  return s;
}
std::string& rtrim(std::string& s, const char* t)
{
  s.erase(s.find_last_not_of(t) + 1);
  return s;
}
std::string& trim(std::string& s, const char* t)
{
  return ltrim(rtrim(s, t), t);
}

std::vector<std::string> split(const std::string& str, const std::vector<std::string>& delims)
{
  std::vector<std::string> parts;

  if (delims.empty())
  {
    parts.push_back(str);
    return parts;
  }

  // auto range = std::views::split(str, delims[0]); // C++20
  std::vector<std::string> range;
  size_t                   start = 0, end = 0;
  while ((end = str.find(delims[0], start)) != std::string::npos)
  {
    range.push_back(str.substr(start, end - start));
    start = end + 1;  // 구분자를 건너뜁니다.
  }
  range.push_back(str.substr(start));

  for (auto&& part : range)
  {
    std::string s;
    for (auto&& c : part)
      s.push_back(c);

    if (s.empty() == false)
    {
      auto subParts = split(s, std::vector<std::string>(delims.begin() + 1, delims.end()));
      parts.insert(parts.end(), subParts.begin(), subParts.end());
    }
  }

  return parts;
}

std::string safe_filename(const std::string& org, const bool allow_single_quote)
{
  if (org.empty())
    return "";

  std::string safe;
  safe.reserve(org.size());

  for (unsigned char c : org)  // support Unicode
  {
    // 절대 금지 문자들 -> _
    if (c == '/' || c == '\\' || c == ':' || c == '*' ||
        c == '?' || c == '|' || c == '<' || c == '>' || c == '&' ||
        c == ';' || c == '\t' || c == '\n' || c == '\r' || c == '\0' ||
        c <= 31 || c == 127)
    {
      safe += '_';
    }
    else if (c == '\'' || c == '"' || c == '`')
    {
      if (allow_single_quote)
        safe += '\'';  // set single quote if allowed
      else
        safe += '_';
    }
    else if (c == '[' || c == '{')
    {
      safe += '(';
    }
    else if (c == ']' || c == '}')
    {
      safe += ')';
    }
    else
    {
      safe += static_cast<char>(c);
    }
  }

  // replace consecutive underscores with a single underscore
  // (ex. "hello__world" -> "hello_world")
  std::string result;
  bool        prev_underscore = false;
  for (char c : safe)
  {
    if (c == '_')
    {
      if (!prev_underscore)  // only add if the previous character was not an underscore
      {
        result += c;
        prev_underscore = true;
      }
    }
    else
    {
      result += c;
      prev_underscore = false;
    }
  }

  while (!result.empty() && (result.front() == ' ' || result.front() == '_'))
    result.erase(0, 1);
  while (!result.empty() && (result.back() == ' ' || result.back() == '_'))
    result.pop_back();

  // Limit the length to 200 characters
  if (result.length() > 200)
    result = result.substr(0, 200);

  return result;
}

// string
std::string format_buffer(const std::vector<uint8_t>& packet, size_t prefix, size_t suffix)
{
  return format_buffer(packet.data(), packet.size(), prefix, suffix);
}

std::string format_buffer(const uint8_t* data, size_t size, size_t prefix, size_t suffix)
{
  std::ostringstream oss;
  auto               total = size;

  if (total <= (prefix + suffix))
  {
    for (auto i = 0; i < total; ++i)
      oss << rs::format("%02X ", data[i]);
  }
  else
  {
    // prefix 바이트
    for (size_t i = 0; i < prefix; ++i)
      oss << rs::format("%02X ", data[i]);

    // 중간 바이트
    oss << "... ";

    // suffix 바이트
    for (size_t i = total - suffix; i < total; ++i)
      oss << rs::format("%02X ", data[i]);
  }

  return oss.str();
}

}  // namespace rs
