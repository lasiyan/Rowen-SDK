#ifndef RS_VISION_IO_OPENCV_IMAGE_HPP
#define RS_VISION_IO_OPENCV_IMAGE_HPP

#include <filesystem>
#include <opencv2/imgcodecs.hpp>

#include "../type_traits.hpp"

namespace rs {
namespace vision {
namespace io {

class image;  // forward declaration

namespace image_impl {

class opencv
{
  friend class rs::vision::io::image;

  static rs::Mat read(const std::string& filename, int flags)
  {
    return cv::imread(filename, flags);
  }

  static bool write(const std::string& filename, const rs::Mat& img)
  {
    for (int i = 1; i <= 2; ++i)  // try 2 times
    {
      auto res = cv::imwrite(filename, img);
      if (res && std::filesystem::exists(filename))
        return true;
      else
        fprintf(stderr, "unexpected : failed to write image to %s\n", filename.c_str());
    }
    return false;
  }
};

};  // namespace image_impl
};  // namespace io
};  // namespace vision
};  // namespace rs

#endif  // RS_VISION_IO_OPENCV_IMAGE_HPP
