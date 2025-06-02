#pragma once

#include <rowen/vision/core/typedef_point.hpp>

namespace rs {
namespace vision {

template <typename T>
class Line_
{
 public:
  Line_() = default;
  Line_(T _x1, T _y1, T _x2, T _y2) : p1(_x1, _y1), p2(_x2, _y2) {}
  Line_(const Point_<T>& _p1, const Point_<T>& _p2) : p1(_p1), p2(_p2) {}
  Line_(const Line_<T>& l) : p1(l.p1), p2(l.p2) {}

  bool operator==(const Line_<T>& l) const { return p1 == l.p1 && p2 == l.p2; }
  bool operator!=(const Line_<T>& l) const { return !(*this == l); }

  Line_<T>& operator=(const Line_<T>& l)
  {
    p1 = l.p1;
    p2 = l.p2;
    return *this;
  }

  Line_<T>  operator+(const Line_<T>& l) { return Line_<T>(p1 + l.p1, p2 + l.p2); }
  Line_<T>  operator-(const Line_<T>& l) { return Line_<T>(p1 - l.p1, p2 - l.p2); }
  Line_<T>& operator+=(const Line_<T>& l) { return *this = *this + l; }
  Line_<T>& operator-=(const Line_<T>& l) { return *this = *this - l; }

 public:
  template <typename U = int>
  Line_<U> operator*(const Size_<U>& s) const
  {
    return Line_<U>(static_cast<U>(p1.x * s.width),
                    static_cast<U>(p1.y * s.height),
                    static_cast<U>(p2.x * s.width),
                    static_cast<U>(p2.y * s.height));
  }

  template <typename U = int>
  Line_<U>& operator*=(const Size_<U>& s)
  {
    return *this = *this * s;
  }

 public:
  double length() const { return sqrt(pow(p1.x - p2.x, 2) + pow(p1.y - p2.y, 2)); }

 public:
  Point_<T> p1, p2;
};

using Line2i = Line_<int>;
using Line2f = Line_<float>;
using Line2d = Line_<double>;
using Line   = Line2i;

};  // namespace vision
};  // namespace rs
