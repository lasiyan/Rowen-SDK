#include "rowen/utils/twilight.hpp"

#include <chrono>
#include <cmath>
#include <iomanip>

namespace rs {
namespace utils {

double Twilight::latitude_  = Twilight::KOREA_LATITUDE;
double Twilight::longitude_ = Twilight::KOREA_LONGITUDE;
int    Twilight::time_zone_ = Twilight::KOREA_TIME_ZONE;

bool Twilight::initialize(const double& _latitude, const double& _longitude, const int& _time_zone)
{
  bool result = true;

  result &= updateLatitude(_latitude);
  result &= updateLongitude(_longitude);
  result &= updateTimeZone(_time_zone);

  return result;
}

bool Twilight::updateLatitude(const double& _latitude)
{
  if (_latitude < -90 || _latitude > 90)
    return false;

  latitude_ = _latitude;
  return true;
}

bool Twilight::updateLongitude(const double& _longitude)
{
  if (_longitude < -180 || _longitude > 180)
    return false;

  longitude_ = _longitude;
  return true;
}

bool Twilight::updateTimeZone(const int& _time_zone)
{
  if (_time_zone < -12 || _time_zone > 12)
    return false;

  time_zone_ = _time_zone;
  return true;
}

bool Twilight::isDaylight(const uint64_t& seconds_tick_count)
{
  using namespace std::chrono;

  std::time_t time_value;
  if (seconds_tick_count)
    time_value = static_cast<std::time_t>(seconds_tick_count);
  else
    time_value = system_clock::to_time_t(system_clock::now());

  time_t sun_rise = GetSunTimeVar(&time_value, true);
  time_t sun_set  = GetSunTimeVar(&time_value, false);

  return (sun_rise <= time_value && time_value <= sun_set);
}

std::time_t Twilight::sunrise()
{
  return GetSunTimeVar(nullptr, true);
}

std::string Twilight::sunriseString(const std::string& format)
{
  return toString(GetSunTimeVar(nullptr, true), format);
}

std::time_t Twilight::sunset()
{
  return GetSunTimeVar(nullptr, false);
}

std::string Twilight::sunsetString(const std::string& format)
{
  return toString(GetSunTimeVar(nullptr, false), format);
}

std::string Twilight::toString(const std::time_t& _time, const std::string& format)
{
  struct tm time_info;
  localtime_r(&_time, &time_info);

  std::ostringstream oss;
  oss << std::put_time(&time_info, format.c_str());

  return oss.str();
}
// --- Calc sunrise/sunset time --------------------------------------------------
std::time_t Twilight::GetSunTimeVar(std::time_t* _date, bool _set_rise)
{
  std::time_t std_time;
  if (_date != nullptr)
    std_time = *_date;
  else
    time(&std_time);

  struct tm loc_time;
  localtime_r(&std_time, &loc_time);

  int luku = -7 * ((loc_time.tm_year + 1900) + ((loc_time.tm_mon + 1) + 9) / 12) / 4 + 275 * (loc_time.tm_mon + 1) / 9 + loc_time.tm_mday;
  luku += (long int)(loc_time.tm_year + 1900) * 367;

  double d      = ((double)luku - 730531.5 + 12.0 / 24.0);
  double L      = FNRange(280.461 * RADS + .9856474 * RADS * d);
  double g      = FNRange(357.528 * RADS + .9856003 * RADS * d);
  double lambda = FNRange(L + 1.915 * RADS * sin(g) + .02 * RADS * sin(2 * g));

  double obliq = 23.439 * RADS - .0000004 * RADS * d;
  double alpha = atan2(cos(obliq) * sin(lambda), cos(lambda));
  double delta = asin(sin(obliq) * sin(lambda));

  double LL = L - alpha;
  if (L < PI)
    LL += T_PI;
  double equation = 1440.0 * (1.0 - LL / T_PI);

  double ha = [](double l, double d) -> double {
    double fo, dfo;
    dfo = RADS * (0.5 * SUN_DIA + AIR_REFR);
    if (l < 0.0)
      dfo = -dfo;
    fo = tan(d + dfo) * tan(l * RADS);
    if (fo > 0.99999)
      fo = 1.0;  // to avoid overflow //
    fo = asin(fo) + PI / 2.0;
    return fo;
  }(latitude_, delta);

  double hb = [](double l, double d) -> double {
    double fi, df1;
    // Correction: different sign at S HS
    df1 = RADS * 6.0;
    if (l < 0.0)
      df1 = -df1;
    fi = tan(d + df1) * tan(l * RADS);
    if (fi > 0.99999)
      fi = 1.0;  // to avoid overflow //
    fi = asin(fi) + PI / 2.0;
    return fi;
  }(latitude_, delta);

  double twx = hb - ha;          // length of twilight in radians
  twx        = 12.0 * twx / PI;  // length of twilight in hours
  // Conversion of angle to hours and minutes //

  double com_val;
  if (_set_rise == true)
    com_val = 12.0 - 12.0 * ha / PI + time_zone_ - longitude_ / 15.0 + equation / 60.0;
  else
    com_val = 12.0 + 12.0 * ha / PI + time_zone_ - longitude_ / 15.0 + equation / 60.0;

  int hour   = (int)com_val;
  int minute = int((com_val - (double)hour) * 60);

  loc_time.tm_hour = hour % 24;
  loc_time.tm_min  = minute % 60;
  loc_time.tm_sec  = 0;

  return std::mktime(&loc_time);
}

// --- Math functions ------------------------------------------------------------
double Twilight::FNRange(const double& x)
{
  double b = x / T_PI;
  double a = T_PI * (b - (long)(b));
  if (a < 0)
    a = T_PI + a;
  return a;
}

}  // namespace utils
}  // namespace rs
