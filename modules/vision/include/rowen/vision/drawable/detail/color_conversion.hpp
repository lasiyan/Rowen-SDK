#pragma once

#include <rowen/vision/type_traits.hpp>

namespace rs::vision {

class ColorConversion
{
 public:
  static rs::Scalar ContrastColor(const rs::Scalar& color, const float threshold = 127.f)
  {
    auto luminance = (color.r * 0.299 + color.g * 0.587 + color.b * 0.114);

    return luminance > threshold ? rs::Scalar(0, 0, 0, color.a) : rs::Scalar(255, 255, 255, color.a);
  }
};

};  // namespace rs::vision
