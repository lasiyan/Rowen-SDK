#include "rowen/core/time.hpp"

#include <cmath>
#include <iomanip>
#include <vector>

using std::chrono::duration_cast;
using std::chrono::hours;

namespace rs {

std::unordered_map<std::string, time::Tick>        time::interval_map_;
std::unordered_map<std::string, std::future<void>> time::async_map_;

// -- methods  -------------------------------------------------------------------

int time::hour()
{
  auto        now = std::chrono::system_clock::now();
  std::time_t t   = std::chrono::system_clock::to_time_t(now);

  std::tm tm = {};
#ifdef _WIN32
  if (localtime_s(&tm, &t) != 0)
    return -1;  // 오류 처리
#else
  if (localtime_r(&t, &tm) == nullptr)
    return -1;  // 오류 처리
#endif
  return tm.tm_hour;
}

std::string time::to_string(Tick tick, const std::string& format, int precision)
{
  seconds     seconds_part;
  nanoseconds fractional_part;

  // Tick 해상도 추정
  if (tick < LIMIT_DIGITS_SECONDS)  // 10자리 미만
  {
    seconds_part    = seconds(tick);
    fractional_part = nanoseconds(0);
  }
  else if (tick < LIMIT_DIGITS_MILLISECONDS)  // 13자리 미만
  {
    seconds_part    = seconds(tick / 1'000);
    fractional_part = nanoseconds((tick % 1'000) * 1'000'000);
  }
  else if (tick < LIMIT_DIGITS_MICROSECONDS)  // 16자리 미만
  {
    seconds_part    = seconds(tick / 1'000'000);
    fractional_part = nanoseconds((tick % 1'000'000) * 1'000);
  }
  else if (tick < LIMIT_DIGITS_NANOSECONDS)  // 19자리 미만
  {
    seconds_part    = seconds(tick / 1'000'000'000);
    fractional_part = nanoseconds(tick % 1'000'000'000);
  }
  else
  {
    return "INVALID_TIME";
    seconds_part    = seconds(0);
    fractional_part = nanoseconds(0);
  }

  // 시간 format을 적용
  std::time_t    time_seconds = seconds_part.count();
  struct std::tm time_info;
#ifdef _WIN32
  localtime_s(&time_info, &time_seconds);
#else
  localtime_r(&time_seconds, &time_info);
#endif

  // 시간 문자열 생성
  std::ostringstream oss;
  oss << std::put_time(&time_info, format.c_str());

  // 소수점 부분 추가
  if (precision > 0)
  {
    if (precision > 9)
      precision = 9;

    // fractional_part를 나노초 단위로 처리하고, precision에 맞게 자릿수를 조정
    oss << '.' << std::setw(precision) << std::setfill('0')
        << (duration_cast<nanoseconds>(fractional_part).count() / static_cast<int>(std::pow(10, 9 - precision)));
  }

  return oss.str();
}

static const std::vector<std::string> time_formats = {
  "%Y-%m-%dT%H:%M:%S",  // 기본 ISO 8601
  "%Y-%m-%d %H:%M:%S",  // 공백 구분
  "%Y/%m/%d %H:%M:%S",  // 슬래시 구분
  "%d-%m-%Y %H:%M:%S",  // 날짜-월-연도
  "%m/%d/%Y %H:%M:%S",  // 월/날짜/연도
  "%Y%m%d_%H%M%S",      // 날짜_시간
  "%Y%m%d %H%M%S",      // 날짜 시간
  "%Y%m%d%H%M%S",       // 날짜시간
  "%Y %m %d %H %M %S"   // 띄어쓰기
};

time::Tick time::from_string(const std::string& org_time_str)
{
  std::tm tm = {};

  // 소수점 부분을 제외한 시간 문자열을 찾아 변환
  auto dot_pos  = org_time_str.find('.');
  auto time_str = (dot_pos == std::string::npos) ? org_time_str : org_time_str.substr(0, dot_pos);

  // 적합한 format을 찾아 시간을 변환
  for (const auto& format : time_formats)
  {
    std::istringstream ss(time_str);
    ss >> std::get_time(&tm, format.c_str());
    if (ss.fail())
      continue;

    // 소수점 부분이 있을 경우, 소수점 부분을 Tick으로 변환
    Tick fraction = 0;
    if (dot_pos != std::string::npos)
    {
      auto fractional_str = org_time_str.substr(dot_pos + 1);  // 1: 소수점은 제외
      if (fractional_str.size() > 9)
        fractional_str = fractional_str.substr(0, 9);  // 9자리 이상은 제거
      else if (fractional_str.empty())
        fractional_str = "0";

      // 소수점 부분을 Tick으로 변환
      fraction = std::stoul(fractional_str) * static_cast<Tick>(std::pow(10, 9 - fractional_str.size()));
    }

    // 시간을 Tick으로 변환
    auto time_point       = std::chrono::system_clock::from_time_t(std::mktime(&tm));
    auto time_since_epoch = time_point.time_since_epoch();

    // 변환된 시간을 Tick으로 변환
    return duration_cast<nanoseconds>(time_since_epoch).count() + fraction;
  }

  // 변환 실패 시, 0을 반환
  return 0;
}

}  // namespace rs

#if RS_TIME_ENABLE_STOPWATCH

std::unordered_map<std::string, time::Tick> time::elapsed_map_;

void time::setTimer(const std::string& key)
{
  auto iter = elapsed_map_.find(key);
  if (iter != elapsed_map_.end())
    iter->second = resolution::tick();
  else
    elapsed_map_.insert({ key, resolution::tick() });
}

void time::removeTimer(const std::string& key)
{
  auto iter = elapsed_map_.find(key);
  if (iter != elapsed_map_.end())
    elapsed_map_.erase(iter);
}

time::Tick time::elapsed(const std::string& key)
{
  auto iter = elapsed_map_.find(key);
  if (iter != elapsed_map_.end())
    return (resolution::tick() - iter->second);
  return 0;
}

#endif

#ifdef RS_TIME_WITH_RESOLUTION
// -- resolution  ----------------------------------------------------------------
time::resolution::type time::resolution::sdk_resolution = time::resolution::type::ms;

time::Tick time::resolution::tick()
{
  auto t = std::chrono::system_clock::now().time_since_epoch();  // 1444555666777888999
  switch (resolution::sdk_resolution)
  {
    case time::resolution::type::sec:
      return static_cast<time::Tick>(duration_cast<seconds>(t).count());  // 1444555666
    case time::resolution::type::ms:
      return static_cast<time::Tick>(duration_cast<milliseconds>(t).count());  // 1444555666777
    case time::resolution::type::us:
      return static_cast<time::Tick>(duration_cast<microseconds>(t).count());  // 1444555666777888
    default:
      return t.count();  // 1444555666777888999 (ns)
  }
}

time::Tick time::resolution::cast(float second)
{
  auto tickValue = resolution::value() * second;
  return static_cast<time::Tick>(std::round(tickValue));
}
#endif
