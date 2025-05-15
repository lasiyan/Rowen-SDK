#include "rowen/vision/converter.hpp"

#ifndef rsdk_local_builder
  #include "rowen_config.h"
#endif

#if defined(RS_VISION_CONVERTER_WITH_ARM_NEON)
  #include <arm_neon.h>
#else
  #include <opencv2/imgproc.hpp>
#endif

namespace rs {
namespace vision {
namespace converter {

void BGR2BGRA(const rs::Mat& src, rs::Mat& dst)
{
#if !defined(RS_VISION_CONVERTER_WITH_ARM_NEON)
  cv::cvtColor(src, dst, cv::COLOR_BGR2BGRA);
#else
  if (src.channels() == 4)
  {
    src.copyTo(dst);
    return;
  }

  // NEON 최적화 사용을 위해 4채널 BGRA로 변환하여 저장할 메모리 할당
  rs::Mat tmp(src.rows, src.cols, CV_8UC4);
  int     pixels = src.rows * src.cols;

  const uchar* src_ptr = src.ptr<uchar>();
  uchar*       dst_ptr = tmp.ptr<uchar>();

  int i = 0;

  // SIMD NEON 루프: 한 번에 8픽셀씩 처리
  for (; i <= pixels - 8; i += 8)
  {
    uint8x8x3_t bgr   = vld3_u8(src_ptr);  // 3채널 BGR 로드
    uint8x8_t   alpha = vdup_n_u8(255);    // 알파 채널을 255로 설정

    uint8x8x4_t bgra;
    bgra.val[0] = bgr.val[0];  // B
    bgra.val[1] = bgr.val[1];  // G
    bgra.val[2] = bgr.val[2];  // R
    bgra.val[3] = alpha;       // A

    vst4_u8(dst_ptr, bgra);  // BGRA 저장

    src_ptr += 8 * 3;  // 3채널 8픽셀 이동
    dst_ptr += 8 * 4;  // 4채널 8픽셀 이동
  }

  // 나머지 픽셀 처리 (SIMD로 처리되지 않은 부분)
  for (; i < pixels; ++i)
  {
    dst_ptr[0] = src_ptr[0];  // B
    dst_ptr[1] = src_ptr[1];  // G
    dst_ptr[2] = src_ptr[2];  // R
    dst_ptr[3] = 255;         // A

    src_ptr += 3;
    dst_ptr += 4;
  }

  dst = std::move(tmp);
#endif
}

void BGRA2BGR(const rs::Mat& src, rs::Mat& dst)
{
#if !defined(RS_VISION_CONVERTER_WITH_ARM_NEON)
  cv::cvtColor(src, dst, cv::COLOR_BGRA2BGR);
#else
  if (src.channels() == 3)
  {
    src.copyTo(dst);
    return;
  }

  rs::Mat tmp(src.rows, src.cols, CV_8UC3);
  int     pixels = src.rows * src.cols;

  const uchar* src_ptr = src.ptr<uchar>();
  uchar*       dst_ptr = tmp.ptr<uchar>();

  int i = 0;

  // SIMD NEON 루프: 한 번에 8픽셀씩 처리
  for (; i <= pixels - 8; i += 8)
  {
    uint8x8x4_t bgra = vld4_u8(src_ptr);  // 4채널 BGRA 로드

    uint8x8x3_t bgr;
    bgr.val[0] = bgra.val[0];  // B
    bgr.val[1] = bgra.val[1];  // G
    bgr.val[2] = bgra.val[2];  // R

    vst3_u8(dst_ptr, bgr);  // BGR 저장

    src_ptr += 8 * 4;  // 4채널 8픽셀 이동
    dst_ptr += 8 * 3;  // 3채널 8픽셀 이동
  }

  // 나머지 픽셀 처리 (SIMD로 처리되지 않은 부분)
  for (; i < pixels; ++i)
  {
    dst_ptr[0] = src_ptr[0];  // B
    dst_ptr[1] = src_ptr[1];  // G
    dst_ptr[2] = src_ptr[2];  // R

    src_ptr += 4;
    dst_ptr += 3;
  }

  dst = std::move(tmp);
#endif
}

}  // namespace converter
}  // namespace vision
}  // namespace rs
