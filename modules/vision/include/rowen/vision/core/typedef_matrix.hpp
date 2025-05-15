#pragma once

#include <opencv2/core/mat.hpp>

namespace rs {
namespace vision {

using Mat = cv::Mat;

struct _RawMat
{
  uchar* data = nullptr;
  int    cols = 0;
  int    rows = 0;
};
using RawMat = std::shared_ptr<_RawMat>;

};  // namespace vision
};  // namespace rs
