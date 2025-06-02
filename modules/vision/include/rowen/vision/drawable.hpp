#ifndef RS_VISION_DRAW_DRAWABLE_HPP
#define RS_VISION_DRAW_DRAWABLE_HPP

#include <rowen/vision/drawable/drawable_opencv.hpp>   // IWYU pragma: export
#include <rowen/vision/drawable/drawable_typedef.hpp>  // IWYU pragma: export

#if defined(RSDK_WITH_JETSON_UTILS)
  #include <rowen/vision/drawable/drawable_cuda.hpp>  // IWYU pragma: export
#endif

#ifdef RS_VISION_WITH_PANGO
  #include <rowen/vision/drawable/drawable_pango.hpp>  // IWYU pragma: export
#endif

#include <rowen/vision/matrix_check.h>

namespace rs {
namespace vision {

class drawable
{
  // default
  static constexpr auto DEFAULT_THICK = -2;  // 임의의 음수 값 (실제 사각형, 원형, 선분 등 두께는 모두 -1보다 크기 때문)

 public:
  static constexpr auto FILLED = -1;
  // static constexpr auto VERY_SMALL_FONT = -5.1;
  static constexpr auto SMALL_FONT      = -4.1;  // 변환을 위한 임의의 값
  static constexpr auto MIDDLE_FONT     = -3.1;
  static constexpr auto LARGE_FONT      = -2.1;
  static constexpr auto VERY_LARGE_FONT = -1.1;
  static constexpr auto HUGE_FONT       = -0.1;

  // -- Point --------------------------------------------------------------------
  RS_MATRIX static void circle(const T& img, const Point& center, float radius, const Scalar& color, int thickness = FILLED)
  {
    if constexpr (std::is_same_v<T, Mat>)
    {
      drawable_impl::opencv::circle(img, center, radius, color, thickness);
    }
#if defined(RSDK_WITH_JETSON_UTILS)
    else if constexpr (std::is_same_v<T, GpuMat>)
    {
      drawable_impl::cuda::circle(img, center, radius, color, thickness);
    }
#endif
  }

  RS_MATRIX static void circle(const T& img, const Point2f& center, float radius, const Scalar& color, int thickness = FILLED)
  {
    circle(img, center * img.size(), radius, color, thickness);
  }

  // -- Line ---------------------------------------------------------------------
  RS_MATRIX static void line(const T& img, const Point& pt1, const Point& pt2, const Scalar& color, int thickness = DEFAULT_THICK)
  {
    if constexpr (std::is_same_v<T, Mat>)
    {
      assert_thick(thickness, 2);
      drawable_impl::opencv::line(img, pt1, pt2, color, thickness);
    }
#if defined(RSDK_WITH_JETSON_UTILS)
    else if constexpr (std::is_same_v<T, GpuMat>)
    {
      assert_thick(thickness, 2);
      drawable_impl::cuda::line(img, pt1, pt2, color, thickness);
    }
#endif
  }

  RS_MATRIX static void line(const T& img, const Point2f& pt1, const Point2f& pt2, const Scalar& color, int thickness = DEFAULT_THICK)
  {
    line(img, pt1 * img.size(), pt2 * img.size(), color, thickness);
  }

  RS_MATRIX static void line(const T& img, const Line& _line, const Scalar& color, int thickness = DEFAULT_THICK)
  {
    line(img, _line.p1, _line.p2, color, thickness);
  }

  RS_MATRIX static void line(const T& img, const Line2f& _line, const Scalar& color, int thickness = DEFAULT_THICK)
  {
    line(img, _line.p1 * img.size(), _line.p2 * img.size(), color, thickness);
  }

  // -- Rectangle ----------------------------------------------------------------
  RS_MATRIX static void rectangle(const T& img, const Rect& rect, const Scalar& color, int thickness = DEFAULT_THICK)
  {
    if constexpr (std::is_same_v<T, Mat>)
    {
      assert_thick(thickness, 2);
      drawable_impl::opencv::rectangle(img, rect, color, thickness);
    }
#if defined(RSDK_WITH_JETSON_UTILS)
    else if constexpr (std::is_same_v<T, GpuMat>)
    {
      assert_thick(thickness, FILLED);
      drawable_impl::cuda::rectangle(img, rect, color, thickness);
    }
#endif
  }

  RS_MATRIX static void rectangle(const T& img, const Rect2f& rect, const Scalar& color, int thickness = DEFAULT_THICK)
  {
    rectangle(img, rect * img.size(), color, thickness);
  }

  // -- Polylines ----------------------------------------------------------------
  RS_MATRIX static void polylines(const T& img, const std::vector<Point>& ptis, bool is_closed, const Scalar& color, int thickness = DEFAULT_THICK)
  {
    if constexpr (std::is_same_v<T, Mat>)
    {
      assert_thick(thickness, 2);
      drawable_impl::opencv::polylines(img, ptis, is_closed, color, thickness);
    }
#if defined(RSDK_WITH_JETSON_UTILS)
    else if constexpr (std::is_same_v<T, GpuMat>)
    {
      assert_thick(thickness, 2);
      drawable_impl::cuda::polylines(img, ptis, is_closed, color, thickness);
    }
#endif
  }

  RS_MATRIX static void polylines(const T& img, const std::vector<Point2f>& ptfs, bool is_closed, const Scalar& color, int thickness = DEFAULT_THICK)
  {
    std::vector<Point> ptis;
    for (auto& pt : ptfs)
      ptis.emplace_back(pt * img.size());
    polylines(img, ptis, is_closed, color, thickness);
  }

  // -- Text ---------------------------------------------------------------------
  static Size getTextSize(CString& str, int* baseLine, float font_size = MIDDLE_FONT, int thickness = DEFAULT_THICK, int font = -1)
  {
    assert_font_size(font_size);
    assert_thick(thickness, 2);
    // assert_font_type(font, cv::FONT_HERSHEY_SIMPLEX); // in drawable_impl::opencv
    return drawable_impl::opencv::getTextSize(str, font_size, thickness, font, baseLine);
  }

#if defined(RSDK_WITH_JETSON_UTILS)
  static Size getTextSize(CString& str, const rs::Size& frame_size, float font_size = MIDDLE_FONT)
  {
    assert_font_size_CUDA(font_size, frame_size);
    return drawable_impl::cuda::getTextSize(str, font_size);
  }
#endif

  RS_MATRIX static void text(const T& img, CString& str, const Point& pt, const Scalar& color, float size = MIDDLE_FONT,
                             int thickness = DEFAULT_THICK, int font = -1)
  {
    if (str.empty())
      return;

    if constexpr (std::is_same_v<T, Mat>)
    {
      assert_font_size(size);
      assert_thick(thickness, 2);
      drawable_impl::opencv::text(img, str, pt, color, size, thickness, font);
    }
#if defined(RSDK_WITH_JETSON_UTILS)
    else if constexpr (std::is_same_v<T, GpuMat>)
    {
      assert_font_size_CUDA(size, img.size());
      drawable_impl::cuda::text(img, str, pt, color, size);
    }
#endif
  }

  // -- OSD ----------------------------------------------------------------------
  class osd
  {
   public:
    RS_MATRIX static void setText(const T& img, CString& str, int& spacing, float size = MIDDLE_FONT, int thickness = DEFAULT_THICK)
    {
      if (str.empty())
        return;

      if constexpr (std::is_same_v<T, Mat>)
      {
        assert_font_size(size);
        assert_thick(thickness, 2);
        drawable_impl::opencv::setText(img, str, spacing, size, thickness);
      }
#if defined(RSDK_WITH_JETSON_UTILS)
      else if constexpr (std::is_same_v<T, GpuMat>)
      {
        assert_font_size_CUDA(size, img.size());
        assert_thick(thickness, 2);
        drawable_impl::cuda::setText(img, str, spacing, size, thickness);
      }
#endif
    }

    RS_MATRIX static void setLabel(const T& img, CString& str, const Rect& rect, const Scalar& color)
    {
      if (str.empty())
        return;

      if constexpr (std::is_same_v<T, Mat>)
      {
        int text_thickness = 2;
        drawable_impl::opencv::setLabel(img, str, rect, color, text_thickness);
      }
#if defined(RSDK_WITH_JETSON_UTILS)
      else if constexpr (std::is_same_v<T, GpuMat>)
      {
        float font_size = MIDDLE_FONT;
        assert_font_size_CUDA(font_size, img.size());
        drawable_impl::cuda::setLabel(img, str, rect, color, font_size);
      }
#endif
    }

    RS_MATRIX static void setLabel(const T& img, CString& str, const Rect2f& rect, const Scalar& color)
    {
      setLabel(img, str, rect * img.size(), color);
    }
  };

  // -- Private ------------------------------------------------------------------
 private:
  static void assert_thick(int& thickness, int default_thick)
  {
    if (thickness <= DEFAULT_THICK)
    {
      thickness = default_thick;
    }
  }

  static void assert_font_size(float& font_scale)
  {
    if (font_scale > 0.0)
      return;
    if (font_scale < static_cast<int>(SMALL_FONT))
      font_scale = 0.7;
    else if (font_scale < static_cast<int>(MIDDLE_FONT))
      font_scale = 0.9;
    else if (font_scale < static_cast<int>(LARGE_FONT))
      font_scale = 1.2;
    else if (font_scale < static_cast<int>(VERY_LARGE_FONT))
      font_scale = 1.4;
    else if (font_scale < static_cast<int>(HUGE_FONT))
      font_scale = 2.0;
  }

  static void assert_font_size_CUDA(float& font_size, const rs::Size& size)
  {
    float ratio = (float)size.width / 1920.0;

    if (font_size > 0.0)
      return;
    if (font_size < static_cast<int>(SMALL_FONT))
      font_size = 30.0 * ratio;
    else if (font_size < static_cast<int>(MIDDLE_FONT))
      font_size = 45.0 * ratio;
    else if (font_size < static_cast<int>(LARGE_FONT))
      font_size = 50.0 * ratio;
    else if (font_size < static_cast<int>(VERY_LARGE_FONT))
      font_size = 72.0 * ratio;
    else if (font_size < static_cast<int>(HUGE_FONT))
      font_size = 84.0 * ratio;
  }

#if defined(RSDK_WITH_JETSON_UTILS)
  static bool is_cuda_font(float font)
  {
    // CUDA font는 일반적으로 10.0 이상의 값이다. 반대로 OpenCV는 스케일이기 때문에 1.0 내외의 값이다.
    return (font > 10.0);
  }
#endif
};

};  // namespace vision
};  // namespace rs

#endif  // RS_VISION_DRAW_DRAWABLE_HPP
