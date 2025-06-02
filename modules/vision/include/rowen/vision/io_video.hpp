#ifndef RS_VISION_IO_VIDEO_HPP
#define RS_VISION_IO_VIDEO_HPP

#include <rowen/vision/io/opencv_video.hpp>  // IWYU pragma: export

#if defined(RSDK_WITH_JETSON_UTILS)
  #include <rowen/vision/io/cuda_video.hpp>  // IWYU pragma: export
#endif

namespace rs {
namespace vision {
namespace io {

class video
{
 public:
};

};  // namespace io
};  // namespace vision
};  // namespace rs

#endif  // RS_VISION_IO_VIDEO_HPP
