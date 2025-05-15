#pragma once

#include <chrono>
#include <cmath>
#include <future>
#include <string>
#include <thread>
#include <unordered_map>

using namespace std::chrono_literals;
using std::chrono::microseconds;
using std::chrono::milliseconds;
using std::chrono::nanoseconds;
using std::chrono::seconds;

namespace rs {

template <typename chrono_resolution>
using next_resolution_t = typename std::conditional<
    std::is_same<chrono_resolution, seconds>::value,
    milliseconds,
    typename std::conditional<
        std::is_same<chrono_resolution, milliseconds>::value,
        microseconds,
        nanoseconds>::type>::type;

class time
{
  using Tick  = uint64_t;
  using Value = uint64_t;

  using default_chrono_duration = nanoseconds;  // tick()의 기본 반환 단위

  static constexpr const char* RESOLUTION_TYPE_UNITS[4]  = { "sec", "ms", "us", "ns" };
  static constexpr Tick        LIMIT_DIGITS_SECONDS      = 10'000'000'000ULL;
  static constexpr Tick        LIMIT_DIGITS_MILLISECONDS = 10'000'000'000'000ULL;
  static constexpr Tick        LIMIT_DIGITS_MICROSECONDS = 10'000'000'000'000'000ULL;
  static constexpr Tick        LIMIT_DIGITS_NANOSECONDS  = 10'000'000'000'000'000'000ULL;
  static constexpr Tick        CURRENT_TIME              = 0;

 public:
  /**
   */
  template <typename chrono_duration>
  static constexpr const char* unit()
  {
    if constexpr (std::is_same_v<chrono_duration, seconds>)
      return "sec";
    else if constexpr (std::is_same_v<chrono_duration, milliseconds>)
      return "ms";
    else if constexpr (std::is_same_v<chrono_duration, microseconds>)
      return "us";
    else if constexpr (std::is_same_v<chrono_duration, nanoseconds>)
      return "ns";
    return "undefined";
  }

  /**
   * @brief system_clock 값을 반환한다. (nanoseconds)
   */
  static Tick tick()
  {
    return std::chrono::system_clock::now().time_since_epoch().count();
  }

  /**
   * @brief chrono_duration에 해당하는 system_clock 값을 반환한다.
   */
  template <typename chrono_duration>
  static Tick tick()
  {
    return tickTo<chrono_duration>(tick());
  }

  /**
   * @brief system_clock 값을 chrono_duration에 맞게 변환한다.
   */
  template <typename chrono_duration>
  static Tick tickTo(const Tick system_clock_tick)
  {
    // 범위에 따라 자릿수를 추정하고 변환
    if (system_clock_tick < LIMIT_DIGITS_SECONDS)
      return std::chrono::duration_cast<chrono_duration>(std::chrono::seconds(system_clock_tick)).count();
    else if (system_clock_tick < LIMIT_DIGITS_MILLISECONDS)
      return std::chrono::duration_cast<chrono_duration>(std::chrono::milliseconds(system_clock_tick)).count();
    else if (system_clock_tick < LIMIT_DIGITS_MICROSECONDS)
      return std::chrono::duration_cast<chrono_duration>(std::chrono::microseconds(system_clock_tick)).count();
    else if (system_clock_tick < LIMIT_DIGITS_NANOSECONDS)
      return std::chrono::duration_cast<chrono_duration>(std::chrono::nanoseconds(system_clock_tick)).count();
    return 0;
  }

  /**
   * @brief 현재 시스템의 Hour 값을 반환한다.
   */
  static int hour();

  /**
   * @brief 명시된 시간이 경과했는지 여부를 반환한다.
   * @param chrono_literals : 시간 단위 (ex. 500ms, 1s, 100us 등)
   * @param start : 시작 시간
   * @param end : 종료 시간 (default : 현재 시간)
   */
  template <typename literals>
  static bool elapsed(literals chrono_literals, Tick start, Tick end = CURRENT_TIME)
  {
    // 두 시간을 가장 정말한 해상도로 변환하여 비교
    start = tickTo<nanoseconds>(start);
    end   = (end == CURRENT_TIME) ? tick<nanoseconds>() : tickTo<nanoseconds>(end);

    return (end - start > std::chrono::duration_cast<nanoseconds>(chrono_literals).count());
  }

  static bool elapsed(const float seconds, Tick start, Tick end = CURRENT_TIME)
  {
    return elapsed(convert(seconds), start, end);
  }

  /**
   * @brief 두 시간 간격을 지정된 chrono_duration으로 변환한다.
   * @param start : 시작 시간
   * @param end : 종료 시간 (default : 현재 시간)
   */
  template <typename chrono_duration>
  static Tick elapse(Tick start, Tick end = CURRENT_TIME)
  {
    end = (end == CURRENT_TIME) ? tick<chrono_duration>() : tickTo<chrono_duration>(end);

    return end - tickTo<chrono_duration>(start);
  }

  /**
   * @brief 두 시간 간격을 string으로 반환한다.
   * @param unit_duration : 시간 단위 (ex. seconds, milliseconds, microseconds, nanoseconds)
   * @param start : 시작 시간
   * @param end : 종료 시간 (default : 현재 시간)
   */
  template <typename unit_duration = seconds>
  static std::string elapse_str(Tick start, Tick end = CURRENT_TIME, bool with_unit = true)
  {
    using next_resolution = next_resolution_t<unit_duration>;

    int  precision = 3;
    auto elapsed   = static_cast<long double>(elapse<next_resolution>(start, end)) / std::pow(10, precision);

    if constexpr (std::is_same_v<next_resolution, nanoseconds>)
      precision = 0;

    // printf to string
    char buffer[32];
    if (with_unit)
      snprintf(buffer, sizeof(buffer), "%.*Lf %s", precision, elapsed, unit<unit_duration>());
    else
      snprintf(buffer, sizeof(buffer), "%.*Lf", precision, elapsed);

    return std::string(buffer);
  }

  /**
   * @brief 지정된 시간을 string으로 반환한다.
   * @param tick : 변환할 system clock 값
   * @param format : 변환할 format (default : "%F %T")
   * @param precision : 소수점 자릿수 (default : 0)
   */
  static std::string to_string(Tick tick, const std::string& format = "%F %T", int precision = 0);
  static std::string to_string(Tick tick, int precision) { return to_string(tick, "%F %T", precision); }

  /**
   * @brief 현재 시간을 string으로 반환한다.
   * @param format : 변환할 format (default : "%F %T")
   * @param precision : 소수점 자릿수 (default : 0)
   */
  static std::string timeString(const std::string& format = "%F %T", int precision = 0)
  {
    return to_string(tick(), format, precision);
  }

  /**
   * @brief 시간 문자열을 system_clock 값으로 변환한다.
   * @param time_str : 시간 문자열
   */
  static Tick from_string(const std::string& time_str);

  /**
   * @brief float 형식의 '초'를 chrono duration으로 변환한다.
   */
  template <typename chrono_duration = default_chrono_duration>
  static chrono_duration convert(float seconds)
  {
    return std::chrono::duration_cast<chrono_duration>(std::chrono::duration<float>(seconds));
  }

  /**
   * @brief System clock의 편차를 target_duration의 count로 변환한다.
   */
  template <typename target_duration, typename calculated_tick_duration = default_chrono_duration>
  static Tick count(Tick calculated_tick)
  {
    auto source_time = std::chrono::duration<Tick, typename calculated_tick_duration::period>(calculated_tick);

    // source_duration -> target_duration으로 변환
    return std::chrono::duration_cast<target_duration>(source_time).count();
  }

  /**
   * @brief 시간 간격을 체크하고, 주기적으로 함수를 실행한다.
   * @param interval : 실행 주기
   * @param key : 실행할 함수
   * @param _async : 비동기 실행 여부
   * @param func : 실행할 함수
   * @param args : 함수 인자
   */
  template <typename literals, typename callable, typename... Args>
  static void runInterval(literals interval, const std::string& key, bool _async,
                          callable&& func, Args&&... args)
  {
    auto current_time = tick();

    auto iter = interval_map_.find(key);
    if (iter != interval_map_.end())
    {
      if (elapsed(interval, iter->second, current_time) == true)
      {
        if (_async)
        {
          // 표준 C++ 스레드에서 무한루프에 빠지는 대상은 관리가 불가능하기 때문에
          // 플래그 사용 또는 타임 아웃을 통해 반드시 종료되도록 설계해야 한다.
          if (auto a_iter = async_map_.find(key); a_iter != async_map_.end())
          {
            if (a_iter->second.wait_for(1s) == std::future_status::timeout)
            {
              fprintf(stderr, "***** fatal error : task blocking\n  => %s\n", key.c_str());
              async_map_.erase(a_iter);
            }
          }
          async_map_[key] = std::async(std::launch::async, func, std::forward<Args>(args)...);
        }
        else
        {
          func(std::forward<args>(args)...);
        }
        iter->second = current_time;
      }
    }
    else
    {
      interval_map_.insert({ key, current_time });
    }
  }

  template <typename literals>
  static void sleep(literals time)
  {
    std::this_thread::sleep_for(time);
  }

  static void sleep(const float seconds)
  {
    sleep(std::chrono::duration<float>(seconds));
  }

  /**
   * @brief synchronize method. (wait for locker)
   * @param locker : std::atomic_bool (if false, wait for true)
   * @param timeout : std::chrono::duration literals (like. 10ms, 1s, 100us etc.)
                      zero means infinite wait
   * @return true : success, false : timeout
   */
  template <typename literals = std::chrono::seconds>
  static bool synchronize(std::atomic_bool& locker, literals timeout = 0s)
  {
    auto start = tick();
    while (locker.load() == false)
    {
      if (timeout.count() != 0 && elapsed(timeout, start) == true)  // timeout
        return false;
      rs::time::sleep(5us);
    }
    return true;
  }

 private:
  static std::unordered_map<std::string, Tick>              interval_map_;
  static std::unordered_map<std::string, std::future<void>> async_map_;
};

};  // namespace rs

#ifdef RS_TIME_WITH_RESOLUTION
public:
struct resolution
{
  // clang-format off
     enum class type { sec, ms, us, ns };
  // clang-format on

  static type        sdk_resolution;
  static void        set(type new_sdk_resolution) { sdk_resolution = new_sdk_resolution; }
  static type        get() { return sdk_resolution; }
  static const char* unit() { return RESOLUTION_TYPE_UNITS[static_cast<int>(sdk_resolution)]; }

  /**
   * @brief rs::time의 sdk_resolution에 해당하는 system_clock 값을 반환한다.
   */
  static Tick tick();

  /**
   * @brief seconds(1)를 기준으로, resolution에 해당하는 값 반환
   * @details 예시. resolution이 us라면, 10^6을 반환한다.
   */
  static Value value(type _sdk_resolution = sdk_resolution) { return static_cast<uint64_t>(std::pow(10, 3 * static_cast<int>(_sdk_resolution))); }

  /**
   * @brief '초' 단위를 rs::time의 <현재 resolution>에 해당하는 단위로 변환한다.
   * @example 예시. second: 3.12초, 현재 resolution이 us라면, 3.12 * 10^6 = 3120000를 반환한다.
   */
  static Tick cast(float second);

  /**
   * @brief 현재 resolution의 tick 값을 chrono_duration 형식에 맞게 변환한다.
   * @example
      예시1. 현재 resolution이 ms일 때,
        cast<seconds>(31) => 0  // 31ms는 0초
        cast<microseconds>(31) => 31000  // 31ms는 31000us
        cast<seconds>(45200) => 45  // 45200ms는 45초
        cast<microseconds>(45200) => 45200000  // 45200ms는 45200000us

      예시2. 현재 resolution이 us일 때,
        cast<seconds>(31) => 0  // 31us는 0초
        cast<milliseconds>(31) => 0  // 31us는 0ms
        cast<seconds>(45200) => 0  // 45200us는 0초
        cast<milliseconds>(45200) => 45  // 45200us는 45ms
        cast<nanoseconds>(45200) => 45200  // 45200us는 45200ns

    * @note rs::time::tickTo와 차이점.
      - tickTo는 sdk_resolution과 상관 없이, 명시된 chrono_duration에 맞게 변환한다.
        ex. 1731164348621701 us -> tickTo<seconds> = 1731164348

      - cast는 sdk_resolution에 해당하는 값으로 변환한다.
        cast는 두 tick의 차이를 현재 resolution에 맞게 변환할 때 사용한다.
        ex. t1 = rs::time::tick<milliseconds>();
            // 3초 후
            t2 = rs::time::tick<milliseconds>();
            elapsed = rs::time::resolution::cast<microseconds>(t2 - t1); // 3000ms -> 3000000us
   */
  template <typename chrono_duration>
  static Tick cast(Tick tick_count)
  {
    auto cast_resolution_type = duration_to_custom_resolution<chrono_duration>();
    if (sdk_resolution == cast_resolution_type)
      return tick_count;

    auto cast_resolution_value = value(cast_resolution_type);
    return tick_count * (cast_resolution_value / (long double)value());
  }
};

private:
template <typename chrono_duration>
static constexpr resolution::type duration_to_custom_resolution()
{
  if constexpr (std::is_same_v<chrono_duration, seconds>)
    return resolution::type::sec;
  else if constexpr (std::is_same_v<chrono_duration, milliseconds>)
    return resolution::type::ms;
  else if constexpr (std::is_same_v<chrono_duration, microseconds>)
    return resolution::type::us;
  else if constexpr (std::is_same_v<chrono_duration, nanoseconds>)
    return resolution::type::ns;
  else
    return resolution::sdk_resolution;
}

// get adjust integer value (based on current resolution)
template <typename literals>
static Tick literal_to_current_resolution_tick(literals time)
{
  using std::chrono::duration_cast;
  switch (resolution::sdk_resolution)
  {
    case resolution::type::sec:
      return duration_cast<seconds>(time).count();
    case resolution::type::ms:
      return duration_cast<milliseconds>(time).count();
    case resolution::type::us:
      return duration_cast<microseconds>(time).count();
    case resolution::type::ns:
      return duration_cast<nanoseconds>(time).count();
    default:
      return UINT64_MAX;
  }
}
#endif
