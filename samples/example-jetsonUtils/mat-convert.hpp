#pragma once

#include <cstdio>

#if defined(__platform_jetson__)

  #include <rowen/vision/drawable.hpp>
  #include <rowen/vision/io_image.hpp>

inline void GpuMat_basic(const rs::Mat& cv_mat)
{
  rs::GpuMat gpu_mat(cv_mat);

  // copy test
  auto gpu_mat_clone = gpu_mat.clone();  // Same as gpu_mat
  rs::vision::io::image::write("res_copy_gpu1.jpg", gpu_mat_clone);

  rs::GpuMat gpu_mat_copyTo;
  gpu_mat.copyTo(gpu_mat_copyTo);
  rs::vision::io::image::write("res_copy_gpu2.jpg", gpu_mat_copyTo);

  // reference test
  auto gpu_mat_ref1 = gpu_mat;
  printf("org ref: %d  ref1: %d\n", *gpu_mat.ref, *gpu_mat_ref1.ref);
  {
    auto gpu_mat_ref2 = gpu_mat_ref1;
    printf("org ref: %d  ref1: %d  ref2: %d\n", *gpu_mat.ref, *gpu_mat_ref1.ref, *gpu_mat_ref2.ref);
  }
  printf("org ref: %d  ref1: %d\n", *gpu_mat.ref, *gpu_mat_ref1.ref);
}

inline void GpuMat_draw_save(const rs::Mat& cv_mat)
{
  // CUDA 이미지는 기본 오더가 R G B 이다.
  // 따라서 아래 데이터는 실제로 B G R 순서로 저장된다. (cv::Mat와 역순)
  rs::GpuMat gpu_mat = cv_mat;

  rs::GpuMat gpu_mat_red_rect = gpu_mat.clone();
  // draw 관련 함수에서 Scalar(Color) 순서는 opencv와 호환성을 위해 B G R 순서로 사용한다.
  rs::vision::drawable::rectangle(gpu_mat_red_rect, rs::Rect(400, 400, 300, 300), { 0, 0, 255 }, 3);
  rs::vision::io::image::write("res_draw_red.jpg", gpu_mat_red_rect);  // BLUE image
  // 즉, 0, 0, 255 색상으로 그리면 CV에서는 빨간 사각형이 그려지지만, CUDA에서는 파란 사각형이 그려진다.

  // 만약 GpuMat 의 색상 오더를 B G R로 변경하여 저장하면 cv::Mat와 동일하게 붉은 이미지가 생성된다.
  rs::GpuMat gpu_mat_bgr;
  gpu_mat_red_rect.toBGR(gpu_mat_bgr);                                // convert to B G R order
  auto& gpu_mat_rgb = gpu_mat_red_rect;                               // reference  R G B order
  rs::vision::io::image::write("res_draw_red_bgr.jpg", gpu_mat_bgr);  // RED image

  // 그러나,
  // 만약 CV로 변환한다고 가정하면, CUDA의 R G B 오더는 다시 B G R로 변환된다.
  // R G B 형식의 CUDA 이미지가 GpuMat에서 cv::Mat로 변환되면 B G R 형식의 cv::Mat이 된다. (반대로도 동일)
  rs::Mat cv_mat_from_rgb;
  gpu_mat_rgb.copyTo(cv_mat_from_rgb);  // from R G B order GpuMat

  rs::Mat cv_mat_from_bgr;
  gpu_mat_bgr.copyTo(cv_mat_from_bgr);                                       // from B G R order GpuMat
  rs::vision::io::image::write("res_cv_from_rgb_gpu.jpg", cv_mat_from_rgb);  // RED image
  rs::vision::io::image::write("res_cv_from_bgr_gpu.jpg", cv_mat_from_bgr);  // BLUE image

  ////////////////////////////////////////////////
  // 결론 :
  // 1. CUDA 이미지를 사용할 때 내부 처리와 상관 없이 B G R 순서로 처리한다. (Color Drawing 순서)
  //      (즉, 기존 opencv와 사용하던 순서 및 코드 동일하게 사용)
  // 2. (사용자 눈에 보여지는) 결과 이미지를 저장할 때,
  //      GPU 이미지를 사용하여 저장할 경우 toBGR() 함수를 호출하여 저장하거나
  //      rs::Mat(cv image)로 변환하여 저장한다.
}
#endif

inline void run()
{
#if defined(__platform_jetson__)
  rs::Mat cv_mat = rs::vision::io::image::read<decltype(cv_mat)>("test.jpg");

  // 기본 테스트
  GpuMat_basic(cv_mat);

  // 그리기 및 저장
  GpuMat_draw_save(cv_mat);
#else
  printf("This is not Jetson platform\n");
#endif
}
