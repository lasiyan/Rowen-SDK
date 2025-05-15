#pragma once

#ifdef RS_VISION_WITH_PANGO
  #include <cairo/cairo.h>
  #include <pango/pangocairo.h>
#endif

#include "../type_traits.hpp"
#include "drawable_typedef.hpp"

namespace rs {
namespace vision {

#ifdef RS_VISION_WITH_PANGO
class PangoDrawable
{
  static constexpr auto DEFAULT_FONT   = "/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc";
  static constexpr auto BASE_FONT_SIZE = 24;

 public:
  static constexpr auto DEFAULT_THICK     = 2;
  static constexpr auto DEFAULT_FONT_SIZE = 0.9;
  static constexpr auto FILLED            = -1;

 public:
  /**
   * @brief PangoCairo 랜더링을 위한 Drawable 객체 생성자
   * @param img 입력 이미지 (3채널 또는 4채널 BGR(A) 이미지)
   * @param realtime_relfection 3채널의 경우 실시간 반영 여부
   * @note 3채널 이미지의 경우 4채널로 변환하여 사용. 변환 시 오버헤드가 크기 때문에 가급적 4채널 이미지 사용 권장
   */
  explicit PangoDrawable(rs::Mat& img, bool realtime_relfection = false);
  ~PangoDrawable();

  /**
   * @brief 랜더링된 데이터를 Source 이미지에 반영
   * @details 3채널 이미지의 경우만 동작하며, 반영 이후 자원을 유지한다.
   */
  void synchronize();

  /**
   * @brief 랜더링된 데이터를 Source 이미지에 반영
   * @details 3채널 이미지의 경우만 동작하며, 반영 이후 자원을 해제한다.
   */
  void release();

  // --- Circle & Point ----------------------------------------------------------
  void circle(const Point& center, float radius, const Scalar& color, int thickness = FILLED);
  void circle(const Point2f& center, float radius, const Scalar& color, int thickness = FILLED);

  // --- Line --------------------------------------------------------------------
  void line(const Point& pt1, const Point& pt2, const Scalar& color, int thickness = DEFAULT_THICK);
  void line(const Point2f& pt1, const Point2f& pt2, const Scalar& color, int thickness = DEFAULT_THICK);
  void line(const Line& _line, const Scalar& color, int thickness = DEFAULT_THICK);
  void line(const Line2f& _line, const Scalar& color, int thickness = DEFAULT_THICK);

  // --- Rectangle ---------------------------------------------------------------
  void rectangle(const Rect& rect, const Scalar& color, int thickness = DEFAULT_THICK);
  void rectangle(const Rect2f& rect, const Scalar& color, int thickness = DEFAULT_THICK);

  // --- Polylines ---------------------------------------------------------------
  void polylines(const std::vector<Point>& ptis, bool is_closed, const Scalar& color, int thickness = DEFAULT_THICK);
  void polylines(const std::vector<Point2f>& ptfs, bool is_closed, const Scalar& color, int thickness = DEFAULT_THICK);

  // --- Text --------------------------------------------------------------------
  struct Text
  {
    Scalar font_color;
    float  font_scale      = DEFAULT_FONT_SIZE;
    int    font_thick      = PANGO_WEIGHT_NORMAL;
    int    fixed_font_size = -1;  // -1: auto, 0: relative(width), 1: fixed
  };
  void text(CString& str, const Point& pt, const Scalar& color, float font_scale = DEFAULT_FONT_SIZE, Size* text_size = nullptr);
  Size getTextSize(CString& str, float font_scale = DEFAULT_FONT_SIZE);
  void centerText(CString& str, const int pos_y, const Text& props, Size* text_size = nullptr);

 public:
  // --- OSD Helper --------------------------------------------------------------
  void osdText(CString& str, int& space, float font_scale = DEFAULT_FONT_SIZE, int alpha = 255, int shadow = 2);
  void osdLabel(CString& str, const Rect& rect, const Scalar& color);
  void osdLabel(CString& str, const Rect2f& rect, const Scalar& color);

 public:
  // --- Properties --------------------------------------------------------------
  void setFontFamily(CString& font_path);
  void setFontSizeRelative(bool relative_width = true);

 private:
  int  pangoFontSize(const float font_scale, const bool relative_width = true) const;
  void setCairoColor(const Scalar& color);
  void reflectToSource();

 private:
  // Polymorphic
  rs::Mat&   input_source_;
  const int  source_channel_;
  const Size source_size_;
  const bool realtime_reflection_;
  rs::Mat    prcess_image_;

  cairo_surface_t* surface_ = nullptr;
  cairo_t*         cr_      = nullptr;

  // Font
  PangoLayout*          layout_             = nullptr;
  PangoFontDescription* font_desc_          = nullptr;
  std::string           font_path_          = DEFAULT_FONT;
  int                   last_font_size_     = 0;     // for font size caching
  bool                  relative_font_size_ = true;  // true: relative with image width, false: absolute
};
#endif

};  // namespace vision

#ifdef RS_VISION_WITH_PANGO
using vision::PangoDrawable;
#endif  // RS_VISION_WITH_PANGO

};  // namespace rs
