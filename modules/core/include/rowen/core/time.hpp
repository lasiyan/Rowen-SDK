#ifndef ROWEN_SDK_TIME_TIME_HPP
#define ROWEN_SDK_TIME_TIME_HPP

#include <chrono>
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

class time
{
  using Tick    = uint64_t;
  using Value   = uint64_t;
  using Format  = std::string;
  using CFormat = const std::string&;

 public:
  struct resolution
  {
    static constexpr Tick second      = 1;
    static constexpr Tick millisecond = 1000;
    static constexpr Tick microsecond = 1000000;
    static constexpr Tick nanosecond  = 1000000000;
    static Tick           current_;

    // @param resolution : second, millisecond, microsecond, nanosecond
    static void set(Tick resolution = resolution::millisecond) { current_ = resolution; }
    static Tick get() { return current_; }
  };

 public:
  // get time unit(ms, ns, us etc.)
  static const char* unit();

  // get system tick count
  static Tick tick(Tick resolution = resolution::current_);

  template <typename chrono_duration>
  static Tick tick()
  {
    return tick(duration_to_resolution<chrono_duration>());
  }

  // set or update start time
  static void setTimer(std::string key);

  // remove start time
  static void removeTimer(std::string key);

  // get elapsed tick count
  static Tick elapse(std::string key);

  // check if the specified time has passed (boolean)
  template <typename __literals>
  static bool elapse(__literals target, Tick start, Tick end = 0)
  {
    if (end == 0)
      end = tick();
    return (end - start > literalToTick(target));
  }

  // get time-string in the specified format
  // if tick is 1, current time is used, but if tick is 0, invalid time
  static Format timeString(CFormat format = "%F %T", Tick tick = 1,
                           bool ext = false);

  // get time-string in the fixed format (YYYY-MM-DD HH:mm:SS)
  static Format timeString(Tick tick, bool ext = false);

  // ext resolution time-string (according to chrono duration)
  template <typename chrono_duration>
  static Format timeString()
  {
    return _tickToString<chrono_duration>(time::tick<chrono_duration>(), true, "%F %T");
  }

  // convert second to current resolution tick count
  static Tick casting(double second);

  // Converts the current resolution tick count to the specified resolution
  template <typename chrono_duration>
  static Tick casting(Tick time)
  {
    Tick chrono_resolution = duration_to_resolution<chrono_duration>();

    if (resolution::current_ == chrono_resolution)
      return time;
    else
      return time * (chrono_resolution / (double)resolution::current_);
  }

  // running interval functions
  template <typename literals, typename callable, typename... Args>
  static void runInterval(literals interval, std::string key, bool _async,
                          callable&& func, Args&&... args)
  {
    auto time = tick();
    auto iter = interval_map_.find(key);
    if (iter != interval_map_.end())
    {
      if (time - iter->second > literalToTick(interval))
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
        iter->second = time;
      }
    }
    else
    {
      interval_map_.insert({ key, time });
    }
  }

  template <typename literals>
  static void sleep(literals time)
  {
    std::this_thread::sleep_for(time);
  }

 private:
  // tick count to string
  static Format tickToString(Tick tick, bool ext, CFormat format);

  template <typename chrono_duration>
  static Format _tickToString(Tick tick, bool ext, CFormat format);

  template <typename chrono_duration>
  static constexpr Tick duration_to_resolution()
  {
    if constexpr (std::is_same_v<chrono_duration, seconds>)
      return resolution::second;
    else if constexpr (std::is_same_v<chrono_duration, milliseconds>)
      return resolution::millisecond;
    else if constexpr (std::is_same_v<chrono_duration, microseconds>)
      return resolution::microsecond;
    else if constexpr (std::is_same_v<chrono_duration, nanoseconds>)
      return resolution::nanosecond;
    else
      return 1;
  }

  // get adjust integer value (based on current resolution)
  template <typename literals>
  static Tick literalToTick(literals time)
  {
    using std::chrono::duration_cast;
    switch (resolution::current_)
    {
      case resolution::second:
        return duration_cast<seconds>(time).count();
      case resolution::millisecond:
        return duration_cast<milliseconds>(time).count();
      case resolution::microsecond:
        return duration_cast<microseconds>(time).count();
      case resolution::nanosecond:
        return duration_cast<nanoseconds>(time).count();
      default:
        return UINT64_MAX;
    }
  }

 private:
  static std::unordered_map<std::string, Tick>              elapsed_map_;
  static std::unordered_map<std::string, Tick>              interval_map_;
  static std::unordered_map<std::string, std::future<void>> async_map_;
};

};  // namespace rs

#endif  // ROWEN_SDK_TIME_TIME_HPP
