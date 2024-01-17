#include <cstdarg>
#include <iostream>

#include "rowen/core/time.hpp"

void printf2(const char* format, ...)
{
  va_list args;
  va_start(args, format);
  vprintf(format, args);
  va_end(args);
}

int main()
{
  rs::time::resolution::set(rs::time::resolution::nanosecond);
  auto start_tick = rs::time::tick();
  rs::time::setTimer("my-stop watch-1");

  // second base
  {
    rs::time::resolution::set(rs::time::resolution::second);
    uint64_t e = 21;

    auto to_sec = rs::time::casting<seconds>(e);
    auto to_ms  = rs::time::casting<milliseconds>(e);
    auto to_us  = rs::time::casting<microseconds>(e);
    auto to_ns  = rs::time::casting<nanoseconds>(e);
    printf2("sec to sec : %lu\n", to_sec);
    printf2("sec to milli : %lu\n", to_ms);
    printf2("sec to micro : %lu\n", to_us);
    printf2("sec to nano : %lu\n", to_ns);

    // time string
    printf("time string : %s\n", rs::time::timeString("%F %T", 1, true).c_str());
  }

  // milli base
  {
    rs::time::resolution::set(rs::time::resolution::millisecond);
    uint64_t e = 31234;

    auto to_sec = rs::time::casting<seconds>(e);
    auto to_ms  = rs::time::casting<milliseconds>(e);
    auto to_us  = rs::time::casting<microseconds>(e);
    auto to_ns  = rs::time::casting<nanoseconds>(e);
    printf2("milli to sec : %lu\n", to_sec);
    printf2("milli to milli : %lu\n", to_ms);
    printf2("milli to micro : %lu\n", to_us);
    printf2("milli to nano : %lu\n", to_ns);

    // time string
    printf("time string : %s\n", rs::time::timeString("%F %T", 1, true).c_str());
  }

  // micro base
  {
    rs::time::resolution::set(rs::time::resolution::microsecond);
    uint64_t e = 41234567;

    auto to_sec = rs::time::casting<seconds>(e);
    auto to_ms  = rs::time::casting<milliseconds>(e);
    auto to_us  = rs::time::casting<microseconds>(e);
    auto to_ns  = rs::time::casting<nanoseconds>(e);
    printf2("micro to sec : %lu\n", to_sec);
    printf2("micro to milli : %lu\n", to_ms);
    printf2("micro to micro : %lu\n", to_us);
    printf2("micro to nano : %lu\n", to_ns);

    // time string
    printf("time string : %s\n", rs::time::timeString("%F %T", 1, true).c_str());
  }

  // nano base
  {
    rs::time::resolution::set(rs::time::resolution::nanosecond);
    uint64_t e = 51234567890;

    auto to_sec = rs::time::casting<seconds>(e);
    auto to_ms  = rs::time::casting<milliseconds>(e);
    auto to_us  = rs::time::casting<microseconds>(e);
    auto to_ns  = rs::time::casting<nanoseconds>(e);
    printf2("nano to sec : %lu\n", to_sec);
    printf2("nano to milli : %lu\n", to_ms);
    printf2("nano to micro : %lu\n", to_us);
    printf2("nano to nano : %lu\n", to_ns);

    // time string
    printf("time string : %s\n", rs::time::timeString("%F %T", 1, true).c_str());
  }

  rs::time::resolution::set(rs::time::resolution::nanosecond);
  auto elapsed = rs::time::elapse("my-stop watch-1");

  rs::time::removeTimer("my-stop watch-1");  // optional

  printf("elapsed : %lu\n", elapsed);

  // elapsed check
  bool elapsed_1sec = rs::time::elapse(1s, start_tick);
  printf("elapsed 1sec : %s\n", elapsed_1sec ? "YES" : "NO");

  return 0;
}

int main_sample2()
{
  std::string str;

  // setup (setup time resolution)
  rs::time::resolution::set(rs::time::resolution::millisecond);  // or
  rs::time::resolution::set(rs::time::resolution::microsecond);  // or
  rs::time::resolution::set(rs::time::resolution::nanosecond);

  // ex 1. Get tick count
  auto tick = rs::time::tick();

  // ex 2. Stop-watch
  rs::time::setTimer("my-stop watch-1");
  rs::time::setTimer("my-stop watch-2");

  rs::time::sleep(1s);
  auto elapsed1 = rs::time::elapse("my-stop watch-1");
  printf("elapsed 1 : %lu %s\n", elapsed1, rs::time::unit());

  rs::time::sleep(2s);
  auto elapsed2 = rs::time::elapse("my-stop watch-2");
  printf("elapsed 2 : %lu %s\n", elapsed2, rs::time::unit());

  // ex 3. Time string
  auto tick_time_string     = rs::time::timeString(tick);
  auto curr_time_string     = rs::time::timeString();
  auto tick_time_format_str = rs::time::timeString("%Y%m%d_%H%M%S", tick);
  auto curr_time_format_str = rs::time::timeString("%Y%m%d_%H%M%S");
  auto time_string_ext      = rs::time::timeString(tick, true);  // with millisec

  std::cout << tick_time_string << std::endl;
  std::cout << curr_time_string << std::endl;
  std::cout << tick_time_format_str << std::endl;
  std::cout << curr_time_format_str << std::endl;
  std::cout << time_string_ext << std::endl;

  // ex 4. Get current month or hour (using Time string)
  auto month = std::stoi(rs::time::timeString("%m"));
  auto hour  = std::stoi(rs::time::timeString("%H"));
  printf("current month: %d, hour: %d\n", month, hour);

  // ex 5. Time elased checking (500ms)
  auto start_time = rs::time::tick();
  printf("Elapsed 500ms ?  %s\n",
         rs::time::elapse(500ms, start_time) ? "YES" : "NO");
  rs::time::sleep(500ms);
  printf("Elapsed 500ms ?  %s\n",
         rs::time::elapse(500ms, start_time) ? "YES" : "NO");

  // ex 6. tick count casting (if nano-seconds)
  {
    auto elapsed_time = rs::time::tick() - tick;
    printf("origin time : %lu\n", elapsed_time);

    // case 1. second to "nano-second"
    auto tick = rs::time::casting(3);  // 3 sec is "N" nano seconds
    printf("3 second is %lu nano-seconds\n", tick);

    // case 2. "nano-second" to second
    auto to_sec = rs::time::casting<seconds>(elapsed_time);
    printf("nano to sec : %lu\n", to_sec);

    // case 3. "nano-second" to milli-second
    auto to_ms = rs::time::casting<milliseconds>(elapsed_time);
    printf("nano to milli : %lu\n", to_ms);
  }

  // ex 7. interval function
  auto start = rs::time::tick();
  while (rs::time::elapse(5s, start) == false)
  {
    rs::time::runInterval(1s, "interval_1sec", true,
                          [&] { printf("run interval 1s\n"); rs::time::sleep(1s); });

    rs::time::runInterval(500ms, "interval_500ms_1", false,
                          [&] { printf("run interval 500ms\n"); });

    rs::time::runInterval(500000000ns, "interval_500ms_2", false,
                          [&] { printf("run interval 500000000ns\n"); });
  }

  return 0;
}
