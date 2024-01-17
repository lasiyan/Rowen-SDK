#include "logging-log.h"

void Foo::threadFunc(int repeat)
{
  for (int i = 0; i < repeat; i++)
  {
    logger.fatal("%10s : Repeat %d", __METHOD__, i + 1);
    logger.warn("%10s : This is warn", __METHOD__);
    logger_info("%10s : This is info", __METHOD__);
    logger.debug("%10s : This is debug", __METHOD__);

    logger.trace("%10s : This is trace log", __METHOD__);
    logger.trace("%10s : Class name is %s", __METHOD__, __CLASS__);
    logger.info_raw("%10s : Last Is Raw logging test", __METHOD__);

    usleep(5);
  }
}

void Foo::run(int repeat, const rs::Logger::Property& prop)
{
  logger.setProperties(prop);
  thread_ = std::thread(&Foo::threadFunc, this, repeat);
}
