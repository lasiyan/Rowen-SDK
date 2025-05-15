#ifndef RS_VISION_TYPE_TRAITS_CUDA_HPP
#define RS_VISION_TYPE_TRAITS_CUDA_HPP

#include "core/typedef_cuda_mat.hpp"  // IWYU pragma: export

namespace rs {

using GpuMat = vision::GpuMat;

struct Frame;
struct GpuFrame
{
  GpuMat      img;
  uint64_t    time     = 0;
  std::string error    = "";
  std::string metadata = "";
  int64_t     pos      = -1;
  int         fps      = 0;

  GpuFrame() = default;
  GpuFrame(const Frame& frame);
  GpuFrame& operator=(const Frame& frame);
};

};  // namespace rs

#endif  // RS_VISION_TYPE_TRAITS_CUDA_HPP
