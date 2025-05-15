#ifndef RS_VISION_IO_OPENCV_VIDEO_HPP
#define RS_VISION_IO_OPENCV_VIDEO_HPP

#include <opencv2/videoio.hpp>  // IWYU pragma: export

namespace rs {
namespace vision {
namespace io {

class video;  // forward declaration

namespace video_impl {

class opencv
{
  friend class rs::vision::io::video;
};

};  // namespace video_impl
};  // namespace io
};  // namespace vision
};  // namespace rs

#endif  // RS_VISION_IO_OPENCV_VIDEO_HPP
