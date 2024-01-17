#ifndef ROWEN_SDK_CORE_FUNCTION_HPP
#define ROWEN_SDK_CORE_FUNCTION_HPP

#include <unistd.h>

#include <atomic>
#include <cmath>
#include <string>

namespace rs {

// --- String --------------------------------------------------------------------
std::string& ltrim(std::string& s, const char* t = " \t\n\r\f\v");

std::string& rtrim(std::string& s, const char* t = " \t\n\r\f\v");

std::string& trim(std::string& s, const char* t = " \t\n\r\f\v");

std::string format(const char* fmt, ...);

// --- System --------------------------------------------------------------------
std::string __rowen_terminal_implement(bool sudo, const std::string& cmd);

template <typename... Args>
inline std::string terminal(bool sudo, const char* fmt, Args... args)
{
  return __rowen_terminal_implement(sudo, format(fmt, args...));
}

std::string get_current_path();

std::string get_execute_path();

// atomic alias
template <typename T>
T get_atomic(std::atomic<T>& _var)
{
  return _var.load(std::memory_order_relaxed);
}

template <typename T>
void set_atomic(std::atomic<T>& _var, T _set)
{
  _var.store(_set, std::memory_order_relaxed);
}

// -------------------------------------------------------------------------------
/*
  부동 소수점 연산 오차를 보정하기 위한 함수들
  https://stackoverflow.com/questions/17333/most-effective-way-for-float-and-double-comparison

  (주의) 부동 소수점 숫자를 나타내는 데 일반적으로 사용되는 IEEE 754 표준에서,
  float일반적으로 십진수 약 7자리의 정밀도를 제공하고 double약 15자리의 정밀도를 제공한다.
  즉, float의 경우 소수점 7자리가 초과하지 않으면 0.0 비교 연산에서 정확한 결과를 얻을 수 있고,
  double의 경우 소수점 15자리가 초과하지 않으면 0.0 비교 연산에서 정확한 결과를 얻을 수 있다.

  (참고) https://en.wikipedia.org/wiki/IEEE_754
  (참고) https://en.wikipedia.org/wiki/Single-precision_floating-point_format
*/
template <typename T>
bool is_zero(const T& value, T epsilon = std::numeric_limits<T>::epsilon())
{
  return std::fabs(value) <= epsilon;
}

template <typename T>
bool is_same(const T& v1, const T& v2, T epsilon = std::numeric_limits<T>::epsilon())
{
  return is_zero(v1 - v2, epsilon);
}

// -------------------------------------------------------------------------------
// pretty function
std::string pretty_func_class(const std::string& prettyFunction);

std::string pretty_func_method(const std::string& prettyFunction);

// -------------------------------------------------------------------------------

};  // namespace rs

#ifdef _WIN32
  #define __PRETTY_FUNCTION__ __func__
#endif

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define __CLASS__    (rs::pretty_func_class(__PRETTY_FUNCTION__).c_str())
#define __METHOD__   (rs::pretty_func_method(__PRETTY_FUNCTION__).c_str())

#endif  // ROWEN_SDK_CORE_DEFINE_HPP
