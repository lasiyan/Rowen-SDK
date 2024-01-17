#include "rowen/core/time.hpp"

#include <cmath>

using std::chrono::duration_cast;
using std::chrono::hours;
using std::chrono::system_clock;

namespace rs {

std::unordered_map<std::string, time::Tick>        time::elapsed_map_;
std::unordered_map<std::string, time::Tick>        time::interval_map_;
std::unordered_map<std::string, std::future<void>> time::async_map_;

time::Tick time::resolution::current_ = time::resolution::millisecond;

// -- methods  -------------------------------------------------------------------
const char* time::unit()
{
  switch (resolution::current_)
  {
    case time::resolution::second:
      return "sec";
    case time::resolution::millisecond:
      return "ms";
    case time::resolution::microsecond:
      return "us";
    case time::resolution::nanosecond:
      return "ns";
  }
  return "undefined";
}

time::Tick time::tick(Tick resolution)
{
  auto t = system_clock::now().time_since_epoch();  // 1444555666777888999
  switch (resolution)
  {
    case time::resolution::second:
      return static_cast<time::Tick>(duration_cast<seconds>(t).count());  // 1444555666
    case time::resolution::millisecond:
      return static_cast<time::Tick>(duration_cast<milliseconds>(t).count());  // 1444555666777
    case time::resolution::microsecond:
      return static_cast<time::Tick>(duration_cast<microseconds>(t).count());  // 1444555666777888
    case time::resolution::nanosecond:
      return static_cast<time::Tick>(duration_cast<nanoseconds>(t).count());  // 1444555666777888999
  }
  return 0;
}

void time::setTimer(std::string key)
{
  auto iter = elapsed_map_.find(key);
  if (iter != elapsed_map_.end())
    iter->second = tick();
  else
    elapsed_map_.insert({ key, tick() });
}

void time::removeTimer(std::string key)
{
  auto iter = elapsed_map_.find(key);
  if (iter != elapsed_map_.end())
    elapsed_map_.erase(iter);
}

time::Tick time::elapse(std::string key)
{
  auto iter = elapsed_map_.find(key);
  if (iter != elapsed_map_.end())
    return (tick() - iter->second);
  return 0;
}

time::Format time::timeString(CFormat format, Tick tick, bool ext)
{
  return tickToString(tick, ext, format);
}

time::Format time::timeString(Tick tick, bool ext)
{
  return tickToString(tick, ext, "%F %T");
}

time::Tick time::casting(double second)
{
  auto tickValue = second * resolution::current_;
  return static_cast<time::Tick>(std::round(tickValue));
}

time::Format time::tickToString(Tick tick, bool ext, CFormat format)
{
  if (tick == 0)
  {
    return "INVALID_TIME";
  }
  else if (tick == 1)
  {
    tick = time::tick();
  }

  switch (resolution::current_)
  {
    case resolution::second:
      return _tickToString<seconds>(tick, ext, format);
    case resolution::millisecond:
      return _tickToString<milliseconds>(tick, ext, format);
    case resolution::microsecond:
      return _tickToString<microseconds>(tick, ext, format);
    case resolution::nanosecond:
      return _tickToString<nanoseconds>(tick, ext, format);
  }
  return "";
}

template <typename chrono_duration>
std::string time::_tickToString(Tick tick, bool ext, CFormat format)
{
  using namespace std;
  using time_point = system_clock::time_point;
  using t_duration = time_point::duration;

  time_point  tp{ duration_cast<t_duration>(chrono_duration(tick)) };
  std::time_t t = system_clock::to_time_t(tp);

  std::ostringstream oss;
  struct tm          bt;

#ifdef _WIN32
  localtime_s(&bt, &t);
#else
  localtime_r(&t, &bt);
#endif
  oss << std::put_time(&bt, format.c_str());

  if (ext)
  {
    if (std::is_same<chrono_duration, seconds>())
      oss << '.' << setfill('0') << setw(3) << (tick % resolution::second);  // xx.000 (공백 3자리)
    else if (std::is_same<chrono_duration, milliseconds>())
      oss << '.' << setfill('0') << setw(3) << (tick % resolution::millisecond);
    else if (std::is_same<chrono_duration, microseconds>())
      oss << '.' << setfill('0') << setw(6) << (tick % resolution::microsecond);
    else if (std::is_same<chrono_duration, nanoseconds>())
      oss << '.' << setfill('0') << setw(9) << (tick % resolution::nanosecond);
  }

  return oss.str();
}

}  // namespace rs
