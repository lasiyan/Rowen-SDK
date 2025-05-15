#ifndef RS_VISION_MATRIX_CHECK_HPP
#define RS_VISION_MATRIX_CHECK_HPP

#include "io/opencv_image.hpp"  // IWYU pragma: export

#if defined(RSDK_WITH_JETSON_UTILS)
  #include "io/cuda_image.hpp"  // IWYU pragma: export
#endif

namespace rs {
namespace vision {

#if defined(RSDK_WITH_JETSON_UTILS)
template <typename T>
struct is_matrix_helper
{
  static constexpr bool value = std::is_same_v<T, rs::Mat> || std::is_same_v<T, rs::GpuMat>;
};
#else
template <typename T>
struct is_matrix_helper
{
  static constexpr bool value = std::is_same_v<T, rs::Mat>;
};
#endif

template <typename T>
constexpr bool is_matrix = is_matrix_helper<T>::value;

};  // namespace vision
};  // namespace rs

#define RS_MATRIX template <typename T, typename = std::enable_if_t<is_matrix<T>>>

#endif  // RS_VISION_MATRIX_CHECK_HPP
