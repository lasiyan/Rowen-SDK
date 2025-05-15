#pragma once

#include <unistd.h>

#include <cmath>
#include <cstring>  // IWYU pragma: export // for strrchr on RS_FILENAME
#include <filesystem>
#include <random>
#include <string>
#include <vector>

#include "format.hpp"

namespace rs {

// --- String --------------------------------------------------------------------
std::string& ltrim(std::string& s, const char* t = " \t\n\r\f\v");

std::string& rtrim(std::string& s, const char* t = " \t\n\r\f\v");

std::string& trim(std::string& s, const char* t = " \t\n\r\f\v");

std::string format_buffer(const std::vector<uint8_t>& vec, size_t prefix = 5, size_t suffix = 3);

std::string format_buffer(const uint8_t* data, size_t size, size_t prefix = 5, size_t suffix = 3);

std::vector<std::string> split(const std::string& s, const std::vector<std::string>& delim);

bool containNonASCII(const std::string& str);

// --- System --------------------------------------------------------------------
std::string __platform_terminal_implement(bool sudo, const std::string& cmd);

template <typename... Args>
inline std::string terminal(bool sudo, const char* fmt, Args... args)
{
  return __platform_terminal_implement(sudo, format(fmt, args...));
}

std::filesystem::path get_current_path();
std::filesystem::path get_execute_filepath();
std::filesystem::path get_execute_directory();
std::string           get_execute_directory_string();
std::string           get_execute_filename();

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
  return std::abs(value) <= epsilon;
}

template <typename T>
bool is_same(const T& v1, const T& v2, T epsilon = std::numeric_limits<T>::epsilon())
{
  return is_zero(v1 - v2, epsilon);
}

// -------------------------------------------------------------------------------
// random number generator
template <typename T>
T random(const T range_min, const T range_max)
{
  std::random_device               rd;
  std::mt19937                     gen(rd());
  std::uniform_int_distribution<T> dis(range_min, range_max);
  return dis(gen);
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

#define RS_FILENAME (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#define RS_CLASS    (rs::pretty_func_class(__PRETTY_FUNCTION__).c_str())
#define RS_METHOD   (rs::pretty_func_method(__PRETTY_FUNCTION__).c_str())
