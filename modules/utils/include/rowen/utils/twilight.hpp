#pragma once

#include <cstdint>
#include <ctime>
#include <string>

namespace rs {
namespace utils {

class Twilight
{
  // default value
  static constexpr double KOREA_LATITUDE  = 36.3504;
  static constexpr double KOREA_LONGITUDE = 127.3845;
  static constexpr int    KOREA_TIME_ZONE = 9;

 public:
  /**
   * @brief Initialize the twilight calculator
   * @param latitude  : latitude of the location (-90 ~ 90)
   * @param longitude : longitude of the location (-180 ~ 180)
   * @param time_zone : time zone of the location (-12 ~ 12)
   */
  static bool initialize(const double& latitude  = KOREA_LATITUDE,
                         const double& longitude = KOREA_LONGITUDE,
                         const int&    time_zone = KOREA_TIME_ZONE);

  /**
   * @brief Check if time_tick is daylight
   * @param seconds_tick_count : time tick (0 : automatic set current time)
   * @return true : daylight, false : night
   */
  static bool isDaylight(const uint64_t& seconds_tick_count = 0);

  /**
   * @brief Update the latitude of the location
   * @param latitude : latitude of the location (-90 ~ 90)
   */
  static bool updateLatitude(const double& latitude);

  /**
   * @brief Update the longitude of the location
   * @param longitude : longitude of the location (-180 ~ 180)
   */
  static bool updateLongitude(const double& longitude);

  /**
   * @brief Update the time zone of the location
   * @param time_zone : time zone of the location (-12 ~ 12)
   */
  static bool updateTimeZone(const int& time_zone);

  /**
   * @brief Get the sunrise time
   * @return sunrise time
   */
  static std::time_t sunrise();

  /**
   * @brief Get the sunrise time
   * @param format : time format (default : "%F %T")
   * @return sunrise time string
   */
  static std::string sunriseString(const std::string& format = "%F %T");

  /**
   * @brief Get the sunset time
   * @return sunset time
   */
  static std::time_t sunset();

  /**
   * @brief Get the sunset time
   * @param format : time format (default : "%F %T")
   * @return sunset time string
   */
  static std::string sunsetString(const std::string& format = "%F %T");

 private:
  static std::string toString(const std::time_t& _time, const std::string& format = "%F %T");

  static std::time_t GetSunTimeVar(std::time_t* _date, bool _set_rise);

 private:
  static double FNRange(const double& x);

 private:
  static constexpr double RADS     = 3.14159265358979323846 / 180.0;
  static constexpr double PI       = 3.14159265358979323846;
  static constexpr double T_PI     = 2 * PI;
  static constexpr double SUN_DIA  = 0.53;
  static constexpr double AIR_REFR = 34.0 / 60.0;

  static double latitude_;
  static double longitude_;
  static int    time_zone_;
};

};  // namespace utils
};  // namespace rs
