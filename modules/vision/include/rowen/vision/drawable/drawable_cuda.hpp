#ifndef RS_VISION_DRAWABLE_DRAWABLE_CUDA_HPP
#define RS_VISION_DRAWABLE_DRAWABLE_CUDA_HPP

#include <rowen_jetson/cudaDraw.h>
#include <rowen_jetson/cudaFont.h>

#include <mutex>
#include <rowen/vision/drawable/detail/color_conversion.hpp>
#include <rowen/vision/drawable/drawable_typedef.hpp>
#include <rowen/vision/type_traits.hpp>
#include <rowen/vision/type_traits_cuda.hpp>
#include <unordered_map>

#define DRAWABLE_CUDASYNC()  // CUDA(cudaDeviceSynchronize())

namespace rs {
namespace vision {

class drawable;  // forward declaration

namespace drawable_impl {

class cuda
{
  inline static std::unordered_map<float, cudaFont*> font_table_;
  inline static std::mutex                           font_mutex_;

  friend class rs::vision::drawable;

  using Mat = rs::vision::GpuMat;

  // Point
  static void circle(const Mat& img, const Point& center, float radius, const Scalar& color, int thickness)
  {
    cudaDrawCircle(img.data, img.cols, img.rows, center.x, center.y, radius, toColor(color));
    DRAWABLE_CUDASYNC();
  }

  // Line
  static void line(const GpuMat& img, const Point& pt1, const Point& pt2, const Scalar& color, int thickness)
  {
    cudaDrawLine(img.data, img.cols, img.rows, pt1.x, pt1.y, pt2.x, pt2.y, toColor(color), thickness);
    DRAWABLE_CUDASYNC();
  }

  // Rectangle
  static void rectangle(const GpuMat& img, const Rect& rect, const Scalar& color, int thickness)
  {
    auto inner_color = thickness <= 0 ? toColor(color) : toColor({ 0, 0, 0, 0 });
    auto outer_color = thickness <= 0 ? toColor({ 0, 0, 0, 0 }) : toColor(color);
    thickness        = thickness <= 0 ? 1 : thickness;

    cudaDrawRect(img.data, img.cols, img.rows, rect.x, rect.y,
                 rect.x + rect.width, rect.y + rect.height, inner_color, outer_color, thickness);
    DRAWABLE_CUDASYNC();
  }

  // Polylines
  static void polylines(const GpuMat& img, const std::vector<Point>& ptis, bool is_closed, const Scalar& color, int thickness)
  {
    if (ptis.size() < 2)
      return;

    for (int i = 0; i < ptis.size() - 1; ++i)
      line(img, ptis[i], ptis[i + 1], color, thickness);

    if (is_closed)
      line(img, ptis[ptis.size() - 1], ptis[0], color, thickness);
  }

  // Font
  static Size getTextSize(CString& str, float size)
  {
    std::lock_guard<std::mutex> lock(font_mutex_);

    cudaFont* font = nullptr;
    if (font_table_.find(size) == font_table_.end())
    {
      font = font_table_[size] = cudaFont::Create(size);
    }
    else
    {
      font = font_table_[size];
    }

    if (font == nullptr)
      return Size(0, 0);

    auto font_size = font->TextExtents(str.c_str());
    DRAWABLE_CUDASYNC();

    return Size(font_size.z, font_size.w);
  }

  static void text(const GpuMat& img, CString& str, const Point& org, const Scalar& color, float size)
  {
    std::lock_guard<std::mutex> lock(font_mutex_);

    cudaFont* font = nullptr;
    if (font_table_.find(size) == font_table_.end())
    {
      font = font_table_[size] = cudaFont::Create(size);
    }
    else
    {
      font = font_table_[size];
    }

    if (font)
    {
      font->OverlayText(img.data, img.cols, img.rows, str.c_str(), org.x, org.y, toColor(color));
    }
    DRAWABLE_CUDASYNC();
  }

  // -- OSD ----------------------------------------------------------------------
  static void setText(const GpuMat& img, CString& str, int& line, float size, const Scalar& color, int thick)
  {
    text(img, str, Point(5, line + thick), ColorConversion::ContrastColor(color), size);
    text(img, str, Point(7, line), color, size);

    auto tsize = getTextSize(str, size);
    line += tsize.height;
    line += 10;  // offset
  }

  static void setLabel(const GpuMat& img, CString& str, const Rect& rect, const Scalar& color, float font_size)
  {
    if (rect.x < 0 /* || rect.y < 0*/)
      return;

    Point point;

    int label_margin    = 4;
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
      point.y = (rect.y + rect.height + (text_max_height - label_margin));
    }

    auto tsize = getTextSize(str, font_size);

    Rect label(Point(point.x + 0, point.y - tsize.height),
               Point(point.x + tsize.width, point.y + label_margin));

    // set label background
    rectangle(img, label, color, -1);

    // Adaptive text color
    auto font_color = ColorConversion::ContrastColor(color);

    label.y += (++tsize.height);
    text(img, str, label.tl(), font_color, font_size);
  }

 private:
  static float4 toColor(const Scalar& color)
  {
    // default color order is bgr
    // return make_float4(color.r, color.g, color.b, color.a);
    return make_float4(color.b, color.g, color.r, color.a);
  }

  class FontTableDeleter
  {
   public:
    ~FontTableDeleter()
    {
      std::lock_guard<std::mutex> lock(font_mutex_);

      for (auto& font : font_table_)
      {
        SAFE_DELETE(font.second);
        // DRAWABLE_CUDASYNC();
      }
      font_table_.clear();
    }
  };

  inline static FontTableDeleter font_table_deleter_;
};

};  // namespace drawable_impl
};  // namespace vision
};  // namespace rs

#endif  // RS_VISION_DRAWABLE_DRAWABLE_CUDA_HPP
