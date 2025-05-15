#ifndef RS_VISION_DRAW_CONVERTER_HPP
#define RS_VISION_DRAW_CONVERTER_HPP

#include "type_traits.hpp"

#if defined(__platform_arm64__)
  // #define RS_VISION_CONVERTER_WITH_ARM_NEON
#endif

namespace rs {
namespace vision {
namespace converter {

void BGR2BGRA(const rs::Mat& src, rs::Mat& dst);

void BGRA2BGR(const rs::Mat& src, rs::Mat& dst);

};  // namespace converter
};  // namespace vision
};  // namespace rs

#endif  // RS_VISION_DRAW_CONVERTER_HPP
