#pragma once

#include "typedef_size.hpp"

namespace rs {
namespace vision {

template <typename T>
using Rect_  = cv::Rect_<T>;
using Rect2i = Rect_<int>;
using Rect2f = Rect_<float>;
using Rect2d = Rect_<double>;
using Rect   = Rect2i;

};  // namespace vision
};  // namespace rs

template <typename T, typename U = int>
inline rs::vision::Rect_<U> operator*(const rs::vision::Rect_<T>& r, const rs::vision::Size_<U>& s)
{
  return rs::vision::Rect_<U>(r.x * s.width, r.y * s.height, r.width * s.width, r.height * s.height);
}
