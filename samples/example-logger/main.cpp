#include <rowen/logger.hpp>

// #define LOGGER_BENCHMARK
#ifdef LOGGER_BENCHMARK
  #include <rowen/core/time.hpp>
#endif

int main_simple();

int main()
{
#ifdef LOGGER_BENCHMARK
  auto start_tick = rs::time::tick();
#endif

  logger.setLoggerLevel(rs::LoggerLevel::TRACE);
#ifdef RS_WITH_DATABASE_LOGGER
  logger.activate(rs::LoggerTarget::WEBSVC);
#endif

  logger.log(true, "log with bool log %d success", 123);
  logger.log(false, "log with bool log %d fail", 123);

  RS_LOGGER_FORMAT_TYPE string_msg = "log with string log %d";
  logger.log(true, string_msg, 123);
  logger.log(false, string_msg, 123);

  logger.fatal("fatal log %d", 123);
  logger.info("info log %d", 123);
  logger.debug("debug log %d", 123);
  logger.trace("trace log %d", 123);

  logger_fatal("fatal with tail log %d", 123);
  logger_info("info with tail log %d", 123);
  logger_debug("debug with tail log %d", 123);
  logger_trace("trace with tail log %d", 123);

  logger.console.fatal("fatal console log %d", 123);
  logger.console.info("info console log %d", 123);
  logger.console.debug("debug console log %d", 123);
  logger.console.trace("trace console log %d", 123);

  logger.file.fatal("fatal file log %d", 123);
  logger.file.info("info file log %d", 123);
  logger.file.debug("debug file log %d", 123);
  logger.file.trace("trace file log %d", 123);

  // 한글 문자열 혼합 테스트
  logger.console.fatal("동해물과 백두산이 abcde 12345");
  logger.console.warn("마르고 닳도록 0 1 2 3 4 5 a b c d e");
  logger.console.info("zpqworq ka1mr 하느님이 보우하사 우리나라 만세");
  logger.console.debug("무궁화 삼천리 화려강산");
  logger.console.trace("112 대한 334 사람 556 대한으로 a 길이XXXYTYE보전하세");

  {
    rs::LoggerProperty props(RS_DEBUG_LOG);
    props.using_header_date      = true;
    props.using_micro_resolution = true;
    props.ignore_level           = true;
    logger.log(props, "log with props log %d", 123);
    logger.console.log(props, "log with props console log %d", 123);
    logger.file.log(props, "log with props file log %d", 123);
  }

  {
    rs::LoggerProperty props(RS_ERROR_LOG);
    props.raw_logging = true;
    logger.log(props, "raw log with props log %d", 123);
  }

  // change default logger dir
  logger.file.setDirectory("newer_log");

  // file dir test
  logger.info("dir test : start file log");
  {
    rs::LoggerProperty props(RS_INFO_LOG);
    props.directory = "log2";
    logger.log(props, "dir test : other dir %d", 123);
  }
  logger.info("dir test : last  file log");

  // category test
  logger.info("category test : start file log");
  {
    rs::LoggerProperty props(RS_INFO_LOG);
    props.category = "category2";
    logger.log(props, "category test : other dir %d", 123);
  }
  logger.info("category test : last  file log");

  // both test
  logger.info("both test : start file log");
  {
    rs::LoggerProperty props(RS_INFO_LOG);
    props.directory = "log4/aaa/bbb/ccc";
    props.category  = "category3";
    logger.log(props, "both test : other dir %d", 123);
  }
  logger.info("both test : last  file log");

#ifdef LOGGER_BENCHMARK
  printf("elapsed time : %s\n", rs::time::elapse_str(start_tick).c_str());
#endif

  return 0;
}

int main_simple()
{
  logger.setLoggerLevel(rs::LoggerLevel::TRACE);

  logger.fatal("FATAL 로그 출력 테스트");
  logger.error("ERROR 로그 출력 테스트");
  logger.warn("WARN 로그 출력 테스트");
  logger.info("INFO 로그 출력 테스트");
  logger.debug("DEBUG 로그 출력 테스트");
  logger.trace("TRACE 로그 출력 테스트");

  {
    rs::LoggerProperty props(RS_FATAL_LOG);
    props.raw_logging = true;
    logger.log(props, "FATAL 로그 출력 테스트 2");
  }
  {
    rs::LoggerProperty props(RS_ERROR_LOG);
    props.raw_logging = true;
    logger.log(props, "ERROR 로그 출력 테스트 2");
  }
  {
    rs::LoggerProperty props(RS_WARN_LOG);
    props.raw_logging = true;
    logger.log(props, "WARN 로그 출력 테스트 2");
  }
  {
    rs::LoggerProperty props(RS_INFO_LOG);
    props.raw_logging = true;
    logger.log(props, "INFO 로그 출력 테스트 2");
  }
  {
    rs::LoggerProperty props(RS_DEBUG_LOG);
    props.raw_logging = true;
    logger.log(props, "DEBUG 로그 출력 테스트 2");
  }
  {
    rs::LoggerProperty props(RS_TRACE_LOG);
    props.raw_logging = true;
    logger.log(props, "TRACE 로그 출력 테스트 2");
  }

  {
    rs::LoggerProperty props(RS_FATAL_LOG);
    props.styled_logging = false;
    logger.log(props, "FATAL 로그 출력 테스트 3");
  }
  {
    rs::LoggerProperty props(RS_ERROR_LOG);
    props.styled_logging = false;
    logger.log(props, "ERROR 로그 출력 테스트 3");
  }
  {
    rs::LoggerProperty props(RS_WARN_LOG);
    props.styled_logging = false;
    logger.log(props, "WARN 로그 출력 테스트 3");
  }
  {
    rs::LoggerProperty props(RS_INFO_LOG);
    props.styled_logging = false;
    logger.log(props, "INFO 로그 출력 테스트 3");
  }
  {
    rs::LoggerProperty props(RS_DEBUG_LOG);
    props.styled_logging = false;
    logger.log(props, "DEBUG 로그 출력 테스트 3");
  }
  {
    rs::LoggerProperty props(RS_TRACE_LOG);
    props.styled_logging = false;
    logger.log(props, "TRACE 로그 출력 테스트 3");
  }

  return 0;
}
