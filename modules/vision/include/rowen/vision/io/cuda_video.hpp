#ifndef RS_VISION_IO_CUDA_VIDEO_HPP
#define RS_VISION_IO_CUDA_VIDEO_HPP

#include "rowen_jetson/videoOutput.h"  // IWYU pragma: export

namespace rs {
namespace vision {
namespace io {

class video;  // forward declaration

namespace video_impl {

class cuda
{
  friend class rs::vision::io::video;
};

};  // namespace video_impl
};  // namespace io
};  // namespace vision
};  // namespace rs

#endif  // RS_VISION_IO_CUDA_VIDEO_HPP
