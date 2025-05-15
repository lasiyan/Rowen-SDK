#ifndef RS_VISION_DRAWABLE_DRAWABLE_OPENCV_HPP
#define RS_VISION_DRAWABLE_DRAWABLE_OPENCV_HPP

#include <opencv2/imgproc.hpp>
#include <vector>

#include "../type_traits.hpp"
#include "drawable_typedef.hpp"

namespace rs {
namespace vision {

class drawable;  // forward declaration

namespace drawable_impl {

class opencv
{
  friend class rs::vision::drawable;

  using Mat = rs::Mat;

  // Point
  static void circle(const Mat& img, const Point& center, float radius, const Scalar& color, int thickness)
  {
    cv::circle(img, center, (int)(radius + 0.5), toColor(color), thickness);
  }

  // Line
  static void line(const Mat& img, const Point& pt1, const Point& pt2, const Scalar& color, int thickness)
  {
    cv::line(img, pt1, pt2, toColor(color), thickness, cv::LINE_8);
  }

  // Rectangle
  static void rectangle(const Mat& img, const Rect& rect, const Scalar& color, int thickness)
  {
    cv::rectangle(img, rect, toColor(color), thickness, cv::LINE_8);
  }

  // Polylines
  static void polylines(const Mat& img, const std::vector<Point>& ptis, bool is_closed, const Scalar& color, int thickness)
  {
    if (ptis.size() < 2)
      return;

    cv::polylines(img, ptis, is_closed, toColor(color), thickness, cv::LINE_8);
  }

  // Font
  static Size getTextSize(CString& str, float size, int thickness, int font, int* base_code)
  {
    if (font < 0)
      font = cv::FONT_HERSHEY_SIMPLEX;

    return cv::getTextSize(str, font, size, thickness, base_code);
  }

  static void text(const Mat& img, CString& str, const Point& org, const Scalar& color, float size, int thickness, int font)
  {
    if (font < 0)
      font = cv::FONT_HERSHEY_SIMPLEX;

    cv::putText(img, str, org, font, (double)size, toColor(color), thickness, cv::LINE_8);
  }

  // -- OSD ----------------------------------------------------------------------
  static void setText(const Mat& img, CString& str, int& line, float size, int thick)
  {
    constexpr int  default_thick = 2;
    constexpr int  base_h        = 720;
    constexpr auto font          = cv::HersheyFonts::FONT_HERSHEY_SIMPLEX;

    float scale = (float)img.rows / base_h * size;
    thick *= scale;

    if (thick < default_thick)
      thick = default_thick;

    text(img, str, Point(7, line + thick), Scalar(0, 0, 0), scale, thick, font);
    text(img, str, Point(5, line), Scalar(255, 255, 255), scale, thick, font);

    int  baseline = 0;
    auto tsize    = getTextSize(str, scale, thick, font, &baseline);
    line += (tsize.height + baseline);
  }

  static void setLabel(const Mat& img, CString& str, const Rect& rect, const Scalar& color, int thick)
  {
    if (rect.x < 0 /* || rect.y < 0*/)
      return;

    int    fontface = cv::FONT_HERSHEY_SIMPLEX;
    double scale    = 0.7;
    int    base     = 0;

    Point point;

    int label_margin    = 7;
    int text_max_height = 25;  // text height was maybe 20px

    if (rect.y - text_max_height > 0)
    {
      // set label top-left
      point.x = rect.x;
      point.y = (rect.y - label_margin);
    }
    else
    {
      // set label bottom-left
      point.x = rect.x;
      point.y = (rect.y + rect.height + (text_max_height - label_margin - 2));
    }

    auto tsize = getTextSize(str, scale, thick, fontface, &base);

    Rect label(Point(point.x + 0, point.y + base),
               Point(point.x + tsize.width, point.y - tsize.height));

    // set label background
    rectangle(img, label, color, -1);

    // Adaptive text color
    const auto luminance  = (color.r * 0.299 + color.g * 0.587 + color.b * 0.114);
    const auto threshold  = 127;
    auto       font_color = luminance > threshold ? Scalar(0, 0, 0) : Scalar(255, 255, 255);

    label.y += (++tsize.height);
    text(img, str, label.tl(), font_color, scale, thick, fontface);
  }

 private:
  static cv::Scalar toColor(const Scalar& color)
  {
    return cv::Scalar(color.b, color.g, color.r, color.a);
  }
};

};  // namespace drawable_impl
};  // namespace vision
};  // namespace rs

#endif  // RS_VISION_DRAWABLE_DRAWABLE_OPENCV_HPP
