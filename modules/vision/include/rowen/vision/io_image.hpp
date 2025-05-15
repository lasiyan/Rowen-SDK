#ifndef RS_VISION_IO_IMAGE_HPP
#define RS_VISION_IO_IMAGE_HPP

#include <filesystem>

#include "matrix_check.h"

namespace rs {
namespace vision {
namespace io {

class image
{
 public:
  RS_MATRIX static T read(const std::string& filename)
  {
    if constexpr (std::is_same_v<T, Mat>)
      return image_impl::opencv::read(filename, cv::IMREAD_ANYCOLOR);
#if defined(RSDK_WITH_JETSON_UTILS)
    else if constexpr (std::is_same_v<T, GpuMat>)
      return image_impl::cuda::read(filename, imageFormat::IMAGE_RGB8);
#endif
  }

  static Mat read_cv(const std::string& filename, int flags = cv::IMREAD_ANYCOLOR)
  {
    return image_impl::opencv::read(filename, flags);
  }

#if defined(RSDK_WITH_JETSON_UTILS)
  static GpuMat read_cuda(const std::string& filename, imageFormat format = imageFormat::IMAGE_RGB8)
  {
    return image_impl::cuda::read(filename, format);
  }
#endif

  RS_MATRIX static bool write(const std::string& filename, const T& img)
  {
    assert_directory(filename);

    if constexpr (std::is_same_v<T, Mat>)
      return image_impl::opencv::write(filename, img);
#if defined(RSDK_WITH_JETSON_UTILS)
    if constexpr (std::is_same_v<T, GpuMat>)
      return image_impl::cuda::write(filename, img);
#endif
  }

 private:
  static void assert_directory(const std::string& filename)
  {
    if (auto pos = filename.find_last_of('/'); pos != std::string::npos)
    {
      auto dir = filename.substr(0, pos);
      if (!std::filesystem::exists(dir))
        std::filesystem::create_directories(dir);
    }
  }
};
};  // namespace io
};  // namespace vision
};  // namespace rs

#endif  // RS_VISION_IO_IMAGE_HPP
