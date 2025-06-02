// #if defined(RSDK_WITH_JETSON_UTILS)

#include <rowen/vision/type_traits.hpp>
#include <rowen/vision/type_traits_cuda.hpp>

namespace rs {

Frame& Frame::operator=(const GpuFrame& gpu)
{
#ifdef BUILD_RS_VISION_SHARED
  gpu.img.cvMat(img);
#endif
  time     = gpu.time;
  error    = gpu.error;
  metadata = gpu.metadata;
  pos      = gpu.pos;
  fps      = gpu.fps;

  return *this;
}

GpuFrame& GpuFrame::operator=(const Frame& cv)
{
#ifdef BUILD_RS_VISION_SHARED
  img = cv.img;
#endif
  time     = cv.time;
  error    = cv.error;
  metadata = cv.metadata;
  pos      = cv.pos;
  fps      = cv.fps;

  return *this;
}

#ifdef BUILD_RS_VISION_SHARED
Frame::Frame(const GpuFrame& gpu)
{
  gpu.img.cvMat(img);
  time     = gpu.time;
  error    = gpu.error;
  metadata = gpu.metadata;
  pos      = gpu.pos;
  fps      = gpu.fps;
}

GpuFrame::GpuFrame(const Frame& cv)
{
  img      = cv.img;
  time     = cv.time;
  error    = cv.error;
  metadata = cv.metadata;
  pos      = cv.pos;
  fps      = cv.fps;
}
#endif

}  // namespace rs
