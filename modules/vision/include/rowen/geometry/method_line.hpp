#pragma once

#include <rowen/vision/type_traits.hpp>

namespace rs {
namespace geometry {
namespace method {
namespace line {

// 점과 점 사이 거리 계산
template <typename T>
inline double distance(const Point_<T>& p1, const Point_<T>& p2)
{
  return std::sqrt(((p2.x - p1.x) * (p2.x - p1.x)) + ((p2.y - p1.y) * (p2.y - p1.y)));
}

// 점과 점 사이 거리 계산
template <typename T>
inline double distance(const Line_<T>& line)
{
  return distance(line.p1, line.p2);
}

// 지정된 라인을 프레임 끝까지 연장
template <typename T = int>
inline Line_<T> extension(const Point_<T>& p1, const Point_<T>& p2, const Size_<int>& size)
{
  //    A          B             C
  // (y1-y2)x + (x2-x1)y + (x1y2 - x2y1) = 0
  double A = static_cast<double>(p1.y - p2.y);
  double B = static_cast<double>(p2.x - p1.x);
  double C = static_cast<double>(p1.x * p2.y - p2.x * p1.y);

  if (static_cast<float>(A) == 0.0)
  {
    if constexpr (std::is_same_v<T, int>)
      return { { 0, p1.y }, { size.width, p1.y } };
    else if constexpr (std::is_same_v<T, float>)
      return { { 0, p1.y }, { 1, p1.y } };
  }
  if (static_cast<float>(B) == 0.0)
  {
    if constexpr (std::is_same_v<T, int>)
      return { { p1.x, 0 }, { p1.x, size.height } };
    else if constexpr (std::is_same_v<T, float>)
      return { { p1.x, 0 }, { p1.x, 1 } };
  }

  auto m = /*(-1) * */ (A / B);
  // auto m = (-1) * (A / B);

  Point_<T> ep1(0, 0), ep2(0, 0);

  // size.width / height -> 1. 비율은 최대치가 1
  // 따라서 최대치를 곱하는 연산에서 1로 설정
  if (m < 0)  // bottom-left to top-right
  {
    if constexpr (std::is_same_v<T, int>)
    {
      return Line_<T>((((-1) * (B * 0 + C)) / A), 0,
                      (((-1) * (B * size.height + C)) / A), size.height);
    }
    else if constexpr (std::is_same_v<T, float>)
    {
      return Line_<T>((((-1) * (B * 0 + C)) / A), 0,
                      (((-1) * (B * 1 + C)) / A), 1);
    }
  }

  if (m > 0)  // top-left to bottom-right
  {
    if constexpr (std::is_same_v<T, int>)
    {
      return Line_<T>(0, (((-1) * (A * 0 + C)) / B),
                      size.width, (((-1) * (A * size.width + C)) / B));
    }
    else if constexpr (std::is_same_v<T, float>)
    {
      return Line_<T>(0,
                      (((-1) * (A * 0 + C)) / B),
                      1,
                      (((-1) * (A * 1 + C)) / B));
    }
  }
  return Line_<T>();
}

// 지정된 라인을 프레임 끝까지 연장
inline Line_<int> extension(const Line_<int>& line, const Size_<int>& size)
{
  return extension(line.p1, line.p2, size);
}

// 지정된 라인을 프레임 끝까지 연장
inline Line_<float> extension(const Line_<float>& line, const Size_<int>& size)
{
  return extension(line.p1, line.p2, size);
}

// 라인과 라인 사이의 교차 판정
template <typename T = int>
inline bool intersection(const Point_<T>& p1, const Point_<T>& p2,
                         const Point_<T>& p3, const Point_<T>& p4, Point_<T>* intersect = nullptr)
{
  // int 기준, 4095 * 4095 해상도가 최대치. 그 이상은 float 정수 최대값(16,777,216) 초과로 오버플로우 발생
  float D = ((p1.x - p2.x) * (p3.y - p4.y) - (p1.y - p2.y) * (p3.x - p4.x));

  if (D == 0.0)
    return false;

  auto t = ((p1.x - p3.x) * (p3.y - p4.y) - (p1.y - p3.y) * (p3.x - p4.x)) / D;
  auto u = ((p1.x - p3.x) * (p1.y - p2.y) - (p1.y - p3.y) * (p1.x - p2.x)) / D;

  if ((0 <= t && t <= 1) && (0 <= u && u <= 1))
  {
    if (intersect != nullptr)
    {
      intersect->x = p1.x + t * (p2.x - p1.x);
      intersect->y = p1.y + t * (p2.y - p1.y);
    }
    return true;
  }

  return false;
}

// 라인과 라인 사이의 교차 판정
template <typename T = int>
inline bool intersection(const Line_<T>& l1, const Line_<T>& l2, Point_<T>* intersect = nullptr)
{
  return intersection(l1.p1, l1.p2, l2.p1, l2.p2, intersect);
}

// 라인과 라인 사이의 교차 판정
inline bool intersection(const Line_<float>& l1, const Line_<float>& l2, const Size& size, Point_<int>* intersect = nullptr)
{
  return intersection(l1 * size, l2 * size, intersect);
}

};  // namespace line
};  // namespace method
};  // namespace geometry
};  // namespace rs
