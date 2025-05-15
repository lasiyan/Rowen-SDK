#include "rowen/logger/template/logger_base.hpp"

#include <algorithm>
#include <cassert>
#include <cstring>
#include <ctime>
#include <sstream>
#include <vector>

namespace rs {

bool LoggerBase::assertLevel(LoggerLevel level, const LoggerProperty* props) const
{
  if (props && props->ignore_level)
    return true;

  return (level <= minimal_level_);
}

void LoggerBase::updateInheritProperties(LoggerProperty& props) const
{
  assert(default_props_.using_header_date != LoggerProperty::eLoggerInherit);
  assert(default_props_.using_micro_resolution != LoggerProperty::eLoggerInherit);
  assert(default_props_.raw_logging != LoggerProperty::eLoggerInherit);
  assert(default_props_.ignore_level != LoggerProperty::eLoggerInherit);
  assert(default_props_.styled_logging != LoggerProperty::eLoggerInherit);

  if (props.using_header_date == LoggerProperty::eLoggerInherit)
    props.using_header_date = default_props_.using_header_date;

  if (props.using_micro_resolution == LoggerProperty::eLoggerInherit)
    props.using_micro_resolution = default_props_.using_micro_resolution;

  if (props.raw_logging == LoggerProperty::eLoggerInherit)
    props.raw_logging = default_props_.raw_logging;

  if (props.ignore_level == LoggerProperty::eLoggerInherit)
    props.ignore_level = default_props_.ignore_level;

  if (props.styled_logging == LoggerProperty::eLoggerInherit)
    props.styled_logging = default_props_.styled_logging;
}

void LoggerBase::setHeaderTimestamp(const LoggerProperty& props, Context* context)
{
  // 이미 Timestamp가 설정되어 있으면 무시
  if (context == nullptr || context->header.timestamp[0] != '\0')
    return;

  struct timespec ts;
  struct tm       tm_info;

  clock_gettime(CLOCK_REALTIME, &ts);

  time_t sec = ts.tv_sec;
  localtime_r(&sec, &tm_info);
  context->header.tick = sec * 1000000000 + ts.tv_nsec;

  // micro time (19: YYYY-mm-DD HH:MM:SS, 8: .xxxxxx'\0')
  strftime(context->header.micro_time, sizeof(context->header.micro_time), "%Y-%m-%d %H:%M:%S", &tm_info);
  snprintf(context->header.micro_time + 19, 8, ".%06ld", ts.tv_nsec / 1000);

  // date only (10: YYYY-mm-DD)
  memcpy(context->header.date_only, context->header.micro_time, 10);

  // timestemp
  int tmpos = 0;
  if (props.using_header_date == LoggerProperty::eLoggerTrue)
  {
    const auto DATE_ONLY_SIZE = 10;  //  10: YYYY-mm-DD
    memcpy(context->header.timestamp, context->header.date_only, DATE_ONLY_SIZE);
    tmpos += DATE_ONLY_SIZE;
    context->header.timestamp[tmpos++] = ' ';  // space
  }

  // 12 or 15 : HH:MM:SS.xxx or xxxxxx
  if (props.using_micro_resolution)
  {
    memcpy(context->header.timestamp + tmpos, context->header.micro_time + 11, 15);
  }
  else
  {
    memcpy(context->header.timestamp + tmpos, context->header.micro_time + 11, 12);  // drop last 3 digits
  }
}

void LoggerBase::setHeaderContext(LoggerLevel level, const LoggerProperty& props, Context* context)
{
  if (context == nullptr || !context->header.string.empty())
    return;

  // Fill logging attributes
  if (context->header.timestamp[0] == '\0')
    setHeaderTimestamp(props, context);

  if (context->header.level_string[0] == '\0')
    snprintf(context->header.level_string, sizeof(context->header.level_string), "%-7s", loggerLevelString(level, false));

  // Make header string
  if (props.raw_logging == false)
  {
    char buffer[128] = {};
    snprintf(buffer, sizeof(buffer), "[%s] %-7s", context->header.timestamp, loggerLevelString(level, true));
    context->header.string.assign(buffer);
  }
}

void LoggerBase::setFooterContext(const LoggerProperty& props, Context* context)
{
  if (context == nullptr || !context->footer.string.empty())
    return;

  // Make footer string
  if (props.raw_logging == false)
  {
    if (context->footer.file && context->footer.line > 0)
    {
      char buffer[1024] = {};
      snprintf(buffer, sizeof(buffer), "  ... (%s: %d)", context->footer.file, context->footer.line);
      context->footer.string.assign(buffer);
    }
  }
}

std::string LoggerBase::generateContent(LoggerLevel level, const LoggerProperty& props, Context* context, bool newline)
{
  std::string content;

  // --- Raw logging (or not) ----------------------------------------------------
  if (props.raw_logging)
    content = rs::format("%s", context->body.string.c_str());
  else
    content = rs::format("%s %s %s", context->header.string.c_str(), context->body.string.c_str(), context->footer.string.c_str());

  // --- Styled logging ----------------------------------------------------------
  if (props.styled_logging || (props.style != LOGGER_STYLE_UNSET))
  {
    int style = props.style;

    if (style == LOGGER_STYLE_UNSET)
    {
      switch (level)
      {
        case LoggerLevel::FATAL: style = RS_LOGGER_FATAL_STYLE; break;
        case LoggerLevel::ERROR: style = RS_LOGGER_ERROR_STYLE; break;
        case LoggerLevel::WARN:  style = RS_LOGGER_WARN_STYLE; break;
        case LoggerLevel::INFO:  style = RS_LOGGER_INFO_STYLE; break;
        case LoggerLevel::DEBUG: style = RS_LOGGER_DEBUG_STYLE; break;
        case LoggerLevel::TRACE: style = RS_LOGGER_TRACE_STYLE; break;
        default:                 break;
      }
    }

    // style to escape code string
    auto escape_code_set   = generateEscapeCode(style);
    auto escape_code_reset = generateEscapeCode(LOGGER_STYLE_RESET);

    // set escape code on content
    if (escape_code_set.empty() == false)
      content = rs::format("%s%s%s", escape_code_set.c_str(), content.c_str(), escape_code_reset.c_str());
  }

  // --- Add newline -------------------------------------------------------------
  if (newline && content.empty() == false)
    content.push_back('\n');

  return content;
}

std::string LoggerBase::generateEscapeCode(const int style, const bool for_string_print /*= false*/)
{
  std::vector<int> components;

  // clang-format off
  if (style == LOGGER_STYLE_UNSET)  return "";
  if (style == LOGGER_STYLE_RESET)  components.push_back(0);

  if (style & FONT_STYLE_BOLD)      components.push_back(1);
  if (style & FONT_STYLE_BLUR)      components.push_back(2);
  if (style & FONT_STYLE_ITALIC)    components.push_back(3);
  if (style & FONT_STYLE_UNDERLINE) components.push_back(4);
  if (style & FONT_STYLE_REVERSE)   components.push_back(7);
  if (style & FONT_STYLE_HIDE)      components.push_back(8);
  if (style & FONT_STYLE_STRIKE)    components.push_back(9);

  // 글꼴 색상 (8~15)
  if (style & FONT_COLOR_BLACK)     components.push_back(30);
  if (style & FONT_COLOR_RED)       components.push_back(31);
  if (style & FONT_COLOR_GREEN)     components.push_back(32);
  if (style & FONT_COLOR_YELLOW)    components.push_back(33);
  if (style & FONT_COLOR_BLUE)      components.push_back(34);
  if (style & FONT_COLOR_PURPLE)    components.push_back(35);
  if (style & FONT_COLOR_CYAN)      components.push_back(36);
  if (style & FONT_COLOR_WHITE)     components.push_back(37);

  // 배경색 (16~23)
  if (style & BACKGROUND_BLACK)     components.push_back(40);
  if (style & BACKGROUND_RED)       components.push_back(41);
  if (style & BACKGROUND_GREEN)     components.push_back(42);
  if (style & BACKGROUND_YELLOW)    components.push_back(43);
  if (style & BACKGROUND_BLUE)      components.push_back(44);
  if (style & BACKGROUND_PURPLE)    components.push_back(45);
  if (style & BACKGROUND_CYAN)      components.push_back(46);
  if (style & BACKGROUND_WHITE)     components.push_back(47);

  if (components.empty())           return "";
  // clang-format on

  std::ostringstream oss;
  oss << (for_string_print ? "\\033[" : "\033[");
  for (size_t i = 0; i < components.size(); ++i)
  {
    oss << components[i];
    if (i < components.size() - 1)  // 마지막 요소가 아닌 경우
      oss << ";";
  }
  oss << "m";

  return oss.str();
}

std::optional<LoggerLevel> toLoggerLevel(std::string level)
{
  if (level.empty())
    return std::nullopt;

  // 숫자로 설정 시,
  try
  {
    int num_level = std::stoi(level);

    if (num_level >= static_cast<int>(LoggerLevel::SILENT) && num_level <= static_cast<int>(LoggerLevel::TRACE))
      return static_cast<LoggerLevel>(num_level);
  }
  catch (const std::exception& e)
  {
    // 숫자가 아닌 경우 문자열로 비교
  }

  // 대소문자를 무시하고 문자열 비교
  std::transform(level.begin(), level.end(), level.begin(), ::toupper);

  if (level == "SILENT" || level == "OFF")
    return RS_SILENT_LOG;
  else if (level == "FATAL")
    return RS_FATAL_LOG;
  else if (level == "ERROR")
    return RS_ERROR_LOG;
  else if (level == "WARN")
    return RS_WARN_LOG;
  else if (level == "INFO")
    return RS_INFO_LOG;
  else if (level == "DEBUG")
    return RS_DEBUG_LOG;
  else if (level == "TRACE")
    return RS_TRACE_LOG;

  return std::nullopt;
}

}  // namespace rs
