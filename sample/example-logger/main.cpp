#include "logging-log.h"

static constexpr auto repeat = 3;

int main()
{
  logger.setLevel(rs::Logger::Level::INFO);
  logger.setTarget(rs::Logger::CONSOLE | rs::Logger::FILE);

  std::unique_ptr<Foo>
      test_log1,    // 카테고리, 레벨 변경 테스트
      test_log2,    // TRACE, Header Date, Micro Resolution 테스트
      test_GSAuth;  // 폴더, DB, 카테고리 변경 테스트

  {
    rs::Logger::Property prop;
    prop.category = "system2";
    prop.level    = rs::Logger::Level::DEBUG;
    test_log1     = std::make_unique<Foo>();
    test_log1->run(repeat, prop);
  }

  {
    rs::Logger::Property prop;
    prop.micro_resolution = true;
    prop.set_header_date  = true;
    prop.trace_enable     = false;
    test_log2             = std::make_unique<Foo>();
    test_log2->run(repeat, prop);
  }

  {
    rs::Logger::Property prop;
    prop.level     = rs::Logger::Level::DEBUG;
    prop.directory = "user";
    prop.category  = "user-Category";
    prop.target    = rs::Logger::CONSOLE | rs::Logger::FILE;
    test_GSAuth    = std::make_unique<Foo>();
    test_GSAuth->run(repeat, prop);
  }

  std::thread([&]() {
    for (int i = 0; i < repeat; i++)
    {
      logger.fatal("%10s : Repeat %d", __METHOD__, i + 1);
      logger.warn("%10s : This is warn", __METHOD__);
      logger_info("%10s : This is info", __METHOD__);
      logger.debug("%10s : This is debug", __METHOD__);

      // Change log folder test
      logger.setDirectory("log-change-test");
      logger.info("%10s : change log folder test %s", __METHOD__, logger.directory().c_str());
      logger.resetDirectory();

      logger.trace("%10s : This is trace log", __METHOD__);
      logger.trace("%10s : Class name is %s", __METHOD__, __CLASS__);
      logger.info_raw("%10s : Last Is Raw logging test", __METHOD__);

      usleep(5);
    }
  }).join();

  return 0;
}
