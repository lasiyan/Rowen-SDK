#ifndef RS_VISION_TYPE_TRAITS_HPP
#define RS_VISION_TYPE_TRAITS_HPP

#ifndef rsdk_local_builder
  #include <rowen_config.hpp>
#endif

#include <rowen/vision/core/typedef_line.hpp>     // IWYU pragma: export
#include <rowen/vision/core/typedef_matrix.hpp>   // IWYU pragma: export
#include <rowen/vision/core/typedef_point.hpp>    // IWYU pragma: export
#include <rowen/vision/core/typedef_polygon.hpp>  // IWYU pragma: export
#include <rowen/vision/core/typedef_rect.hpp>     // IWYU pragma: export
#include <rowen/vision/core/typedef_scalar.hpp>   // IWYU pragma: export
#include <rowen/vision/core/typedef_size.hpp>     // IWYU pragma: export

namespace rs {

// Size
template <typename T>
using Size_  = vision::Size_<T>;
using Size2i = Size_<int>;
using Size2f = Size_<float>;
using Size2d = Size_<double>;
using Size   = Size2i;

// Point
template <typename T>
using Point_  = vision::Point_<T>;
using Point2i = Point_<int>;
using Point2f = Point_<float>;
using Point2d = Point_<double>;
using Point   = Point2i;

// Rect
template <typename T>
using Rect_  = vision::Rect_<T>;
using Rect2i = Rect_<int>;
using Rect2f = Rect_<float>;
using Rect2d = Rect_<double>;
using Rect   = Rect2i;

// Line
template <typename T>
using Line_  = vision::Line_<T>;
using Line2i = Line_<int>;
using Line2f = Line_<float>;
using Line2d = Line_<double>;
using Line   = Line2i;

// Scalar
template <typename T>
using Scalar_  = vision::Scalar_<T>;
using Scalar2i = Scalar_<int>;
using Scalar2f = Scalar_<float>;
using Scalar   = Scalar2f;

// Polygon
using Polygon = vision::Polygon;

// Matrix & Frame
using Mat    = vision::Mat;
using RawMat = vision::RawMat;

};  // namespace rs

#endif  // RS_VISION_TYPE_TRAITS_HPP
