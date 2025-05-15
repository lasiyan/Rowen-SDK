#include "rowen/logger.hpp"

namespace rs {

#define RETURN_IF_INVALID_TARGET(t)                          \
  if ((t) < LoggerTarget::CONSOLE || (t) >= LoggerTargetMax) \
    return;

#define RETURN_IF_INVALID_TARGET_WITH_VALUE(t, v)            \
  if ((t) < LoggerTarget::CONSOLE || (t) >= LoggerTargetMax) \
    return v;

void Logger::activate(const LoggerTarget target)
{
  RETURN_IF_INVALID_TARGET(target);

  if (loggers_[target] != nullptr)
    return;

  switch (target)
  {
    case LoggerTarget::CONSOLE: loggers_[target] = std::make_unique<ConsoleLogger>(); break;
    case LoggerTarget::FILE:    loggers_[target] = std::make_unique<FileLogger>(); break;
#ifdef RS_WITH_DATABASE_LOGGER
    case LoggerTarget::WEBSVC: loggers_[target] = std::make_unique<WebsvcLogger>(); break;
#endif
    default: break;
  }
}

void Logger::deactivate(const LoggerTarget target)
{
  RETURN_IF_INVALID_TARGET(target);

  if (loggers_[target] == nullptr)
    return;

  loggers_[target].reset();
  loggers_[target] = nullptr;
}

LoggerProperty Logger::getProperty(const LoggerTarget target) const
{
  RETURN_IF_INVALID_TARGET_WITH_VALUE(target, LoggerProperty{});

  if (loggers_[target] == nullptr)
    return LoggerProperty{};

  return loggers_[target]->getProperty();
}

void Logger::setLoggerLevel(const LoggerLevel level)
{
  setProperty(&LoggerBase::setLoggerLevel, level);
}

LoggerLevel Logger::getLoggerLevel(const LoggerTarget target) const
{
  RETURN_IF_INVALID_TARGET_WITH_VALUE(target, LoggerLevel::SILENT);

  if (loggers_[target] == nullptr)
    return LoggerLevel::SILENT;

  return loggers_[target]->getLoggerLevel();
}

};  // namespace rs
