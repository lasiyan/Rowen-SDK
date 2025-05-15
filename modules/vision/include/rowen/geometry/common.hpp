#pragma once

namespace rs {
namespace geometry {

// Point In Polygon Algorithm
enum
{
  PPI_CUSTOM = 0,  // 일반적으로 볼록 다각형에 적합
  PPI_LASCHA = 1   // 오목 다각형에 자주 사용
};

// Intersection Polygon Algorithm
enum
{
  IPA_GENERAL = 0,  // 범용 IOU 계산
  IPA_SHPC    = 1,  // 볼록 다각형에 적합 (속도 측면에서 일반보다 빠름)
};

};  // namespace geometry
};  // namespace rs
