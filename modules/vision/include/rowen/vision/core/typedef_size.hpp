#pragma once

#include <opencv2/core/types.hpp>  // IWYU pragma: export

namespace rs {
namespace vision {

template <typename T>
using Size_  = cv::Size_<T>;
using Size2i = Size_<int>;
using Size2f = Size_<float>;
using Size2d = Size_<double>;
using Size   = Size2i;

};  // namespace vision
};  // namespace rs
