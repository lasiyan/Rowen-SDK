#pragma once

#include <rowen/vision/core/typedef_size.hpp>  // IWYU pragma: export

namespace rs {
namespace vision {

template <typename T>
using Point_  = cv::Point_<T>;
using Point2i = Point_<int>;
using Point2f = Point_<float>;
using Point2d = Point_<double>;
using Point   = Point2i;

};  // namespace vision
};  // namespace rs

template <typename T, typename U = int>
inline rs::vision::Point_<U> operator*(const rs::vision::Point_<T>& r, const rs::vision::Size_<U>& s)
{
  return rs::vision::Point_<U>(r.x * s.width, r.y * s.height);
}
