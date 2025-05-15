#pragma once

#include <cmath>
#include <vector>

namespace rs {
namespace geometry {
namespace math {

inline int ccw(std::pair<int, int> p1, std::pair<int, int> p2,
               std::pair<int, int> p3)
{
  int s = p1.first * p2.second + p2.first * p3.second + p3.first * p1.second;
  s -= (p1.second * p2.first + p2.second * p3.first + p3.second * p1.first);
  if (s > 0)
    return 1;
  if (s == 0)
    return 0;
  return -1;
}

template <typename T>
inline bool diff(T value1, T value2, int place = 6)
{
  auto bias = std::pow(10, place);
  return (int(value1 * bias) != int(value2 * bias));
}

template <typename T>
inline T sum(const std::vector<T>& list)
{
  T sum = { 0 };
  for (const auto& _elem : list) sum += _elem;
  return sum;
}

template <typename T>
inline double average(const std::vector<T>& list)
{
  return ((double)sum(list) / (double)list.size());
}

};  // namespace math
};  // namespace geometry
};  // namespace rs
