#include "rowen/vision/drawable/drawable_pango.hpp"

#include "rowen/core/function.hpp"
#include "rowen/vision/converter.hpp"

namespace rs {
namespace vision {

#ifdef RS_VISION_WITH_PANGO
  #ifdef _DEBUG
    #define ASSERT_RESOURCE(resource) \
      assert(resource);               \
      if (!resource)                  \
        throw std::runtime_error("Resource is null");
  #else
    #define ASSERT_RESOURCE(resource) \
      if (!resource)                  \
        return;
  #endif

// #define VERIFY_PARAMETER(condition) \
  //   if (!(condition))                 \
  //     throw std::runtime_error("Invalid parameter");

  #define VERIFY_PARAMETER(condition) \
    if (!(condition))                 \
      return;

PangoDrawable::PangoDrawable(rs::Mat& img, bool rt_reflection)
    : input_source_(img), source_channel_(img.channels()), source_size_(img.size()), realtime_reflection_(rt_reflection)
{
  assert(source_channel_ == 3 || source_channel_ == 4);

  // 채널이 3인 경우 img로부터 4채널에 해당하는 데이터 생성
  if (source_channel_ == 3)
    converter::BGR2BGRA(img, prcess_image_);
  else
    prcess_image_ = img;

  surface_ = cairo_image_surface_create_for_data(
      prcess_image_.data,
      CAIRO_FORMAT_ARGB32,
      source_size_.width,
      source_size_.height,
      source_channel_ == 3 ? source_size_.width * 4 : img.step);
  ASSERT_RESOURCE(surface_);

  cr_ = cairo_create(surface_);
  ASSERT_RESOURCE(cr_);

  layout_ = pango_cairo_create_layout(cr_);
  ASSERT_RESOURCE(layout_);

  font_desc_ = pango_font_description_from_string(font_path_.c_str());
  ASSERT_RESOURCE(font_desc_);
}

PangoDrawable::~PangoDrawable()
{
  release();
}

void PangoDrawable::synchronize()
{
  reflectToSource();
}

void PangoDrawable::release()
{
  reflectToSource();

  if (cr_)
    cairo_surface_flush(surface_);

  if (font_desc_)
  {
    pango_font_description_free(font_desc_);
    font_desc_ = nullptr;
  }

  if (layout_)
  {
    g_object_unref(layout_);
    layout_ = nullptr;
  }

  if (cr_)
  {
    cairo_destroy(cr_);
    cr_ = nullptr;
  }

  if (surface_)
  {
    cairo_surface_destroy(surface_);
    surface_ = nullptr;
  }

  if (source_channel_ == 3 && prcess_image_.empty() == false)
    prcess_image_.release();
}

// --- Circle & Point ------------------------------------------------------------
void PangoDrawable::circle(const Point& ct, float radius, const Scalar& color, int thick)
{
  ASSERT_RESOURCE(cr_);

  setCairoColor(color);
  cairo_arc(cr_, ct.x, ct.y, radius, 0, 2 * M_PI);
  cairo_set_line_width(cr_, thick);
  thick < 0 ? cairo_fill(cr_) : cairo_stroke(cr_);

  if (realtime_reflection_)
    reflectToSource();
}

void PangoDrawable::circle(const Point2f& ct, float radius, const Scalar& color, int thick)
{
  circle(ct * source_size_, radius, color, thick);
}

// --- Line ---------------------------------------------------------------------
void PangoDrawable::line(const Point& pt1, const Point& pt2, const Scalar& color, int thick)
{
  ASSERT_RESOURCE(cr_);

  setCairoColor(color);
  cairo_set_line_width(cr_, thick);
  cairo_move_to(cr_, pt1.x, pt1.y);
  cairo_line_to(cr_, pt2.x, pt2.y);
  cairo_stroke(cr_);

  if (realtime_reflection_)
    reflectToSource();
}

void PangoDrawable::line(const Point2f& pt1, const Point2f& pt2, const Scalar& color, int thick)
{
  line(pt1 * source_size_, pt2 * source_size_, color, thick);
}

void PangoDrawable::line(const Line& _line, const Scalar& color, int thick)
{
  line(_line.p1, _line.p2, color, thick);
}

void PangoDrawable::line(const Line2f& _line, const Scalar& color, int thick)
{
  line(_line.p1 * source_size_, _line.p2 * source_size_, color, thick);
}

// --- Rectangle -----------------------------------------------------------------
void PangoDrawable::rectangle(const Rect& rect, const Scalar& color, int thick)
{
  ASSERT_RESOURCE(cr_);
  VERIFY_PARAMETER(rect.width > 0 && rect.height > 0);

  setCairoColor(color);
  cairo_set_line_width(cr_, thick);
  cairo_rectangle(cr_, rect.x, rect.y, rect.width, rect.height);
  thick < 0 ? cairo_fill(cr_) : cairo_stroke(cr_);

  if (realtime_reflection_)
    reflectToSource();
}

void PangoDrawable::rectangle(const Rect2f& rect, const Scalar& color, int thick)
{
  rectangle(rect * source_size_, color, thick);
}

// --- Polylines -----------------------------------------------------------------
void PangoDrawable::polylines(const std::vector<Point>& ptis, bool is_closed, const Scalar& color, int thick)
{
  ASSERT_RESOURCE(cr_);
  VERIFY_PARAMETER(ptis.size() > 1);

  setCairoColor(color);
  cairo_set_line_width(cr_, thick);
  cairo_move_to(cr_, ptis[0].x, ptis[0].y);

  for (auto i = 1; i < ptis.size(); ++i)
    cairo_line_to(cr_, ptis[i].x, ptis[i].y);

  if (is_closed)
    cairo_close_path(cr_);

  cairo_stroke(cr_);

  if (realtime_reflection_)
    reflectToSource();
}

void PangoDrawable::polylines(const std::vector<Point2f>& ptfs, bool is_closed, const Scalar& color, int thick)
{
  std::vector<Point> ptis;
  for (auto& pt : ptfs)
    ptis.emplace_back(pt * source_size_);

  polylines(ptis, is_closed, color, thick);
}

// --- Text ----------------------------------------------------------------------
void PangoDrawable::text(CString& str, const Point& pt, const Scalar& color, float fsize, Size* text_size)
{
  ASSERT_RESOURCE(cr_);
  ASSERT_RESOURCE(layout_);
  ASSERT_RESOURCE(font_desc_);
  VERIFY_PARAMETER(fsize > 0);
  VERIFY_PARAMETER(str.empty() == false);

  int font_size = pangoFontSize(fsize, relative_font_size_);
  if (font_size != last_font_size_)
  {
    pango_font_description_set_size(font_desc_, font_size);
    pango_layout_set_font_description(layout_, font_desc_);
    last_font_size_ = font_size;
  }

  setCairoColor(color);
  pango_layout_set_text(layout_, str.c_str(), -1);
  cairo_move_to(cr_, pt.x, pt.y);
  pango_cairo_show_layout(cr_, layout_);

  if (text_size)
  {
    pango_layout_get_pixel_size(layout_, &text_size->width, &text_size->height);

    // 한글인 경우 높이가 너무 크게 나오는 문제가 있어서 높이를 조정
    if (containNonASCII(str))
      text_size->height = text_size->height * 0.85;
  }

  if (realtime_reflection_)
    reflectToSource();
}

Size PangoDrawable::getTextSize(CString& str, float fsize)
{
  if (!layout_ || !font_desc_)
    return Size(0, 0);

  if (str.empty() || fsize <= 0)
    return Size(0, 0);

  int font_size = pangoFontSize(fsize, relative_font_size_);
  if (font_size != last_font_size_)
  {
    pango_font_description_set_size(font_desc_, font_size);
    pango_layout_set_font_description(layout_, font_desc_);
    last_font_size_ = font_size;
  }

  pango_layout_set_text(layout_, str.c_str(), -1);

  Size text_size;
  pango_layout_get_pixel_size(layout_, &text_size.width, &text_size.height);

  // 한글인 경우 높이가 너무 크게 나오는 문제가 있어서 높이를 조정
  if (containNonASCII(str))
    text_size.height = text_size.height * 0.85;

  return text_size;
}

void PangoDrawable::centerText(CString& str, const int pos_y, const Text& props, Size* text_size)
{
  ASSERT_RESOURCE(cr_);
  ASSERT_RESOURCE(layout_);
  ASSERT_RESOURCE(font_desc_);
  VERIFY_PARAMETER(props.font_scale > 0);
  VERIFY_PARAMETER(str.empty() == false);

  bool is_fixed  = (props.fixed_font_size == -1) ? relative_font_size_ : (props.fixed_font_size == 0) ? true
                                                                                                      : false;
  int  font_size = pangoFontSize(props.font_scale, is_fixed);
  if (font_size != last_font_size_)
  {
    pango_font_description_set_size(font_desc_, font_size);
    pango_layout_set_font_description(layout_, font_desc_);
    last_font_size_ = font_size;
  }

  setCairoColor(props.font_color);
  pango_layout_set_text(layout_, str.c_str(), -1);

  // 폰트 사이즈 확인 (가운데 정렬)
  int text_width, text_height;
  pango_layout_get_size(layout_, &text_width, &text_height);
  int x = (source_size_.width - text_width / PANGO_SCALE) / 2;
  int y = pos_y < 0 ? 0 : pos_y;

  cairo_move_to(cr_, x, y);
  pango_cairo_show_layout(cr_, layout_);

  if (text_size)
    pango_layout_get_pixel_size(layout_, &(text_size->width), &(text_size->height));

  if (realtime_reflection_)
    reflectToSource();
}

// --- OSD Helper ----------------------------------------------------------------
void PangoDrawable::osdText(CString& str, int& space, float size, int alpha, int shadow)
{
  Size text_size;
  text(str, Point(5 + shadow, space + shadow), Scalar(0, 0, 0, alpha), size, &text_size);
  text(str, Point(5, space), Scalar(255, 255, 255, alpha), size);
  space += text_size.height;
}

void PangoDrawable::osdLabel(CString& str, const Rect& rect, const Scalar& color)
{
  VERIFY_PARAMETER(str.empty() == false);

  constexpr auto TEXT_FONT_SIZE = 0.7;
  constexpr auto TEXT_X_MARGIN  = 10;

  // --- Label -------------------------------------------------------------------
  // 텍스트 크기 계산
  Size text_size = getTextSize(str, TEXT_FONT_SIZE);

  // 라벨 박스 위치 및 크기 설정
  rs::Rect label(rect.x, 0, text_size.width + TEXT_X_MARGIN * 2, text_size.height);  // 2: 좌우 여백

  // Y 위치 보정 (화면 상단 객체인 경우 텍스트가 화면 밖으로 벗어나는 경우가 있음)
  if (rect.y - text_size.height <= 0)  // 화면 위쪽 객체인 경우
    label.y = rect.y + rect.height;    // 객체 하단에 라벨 박스 표시
  else
    // 1: 박스랑 너무 붙어서 여백 추가
    label.y = rect.y - text_size.height - 1;  // 객체 상단에 라벨 박스 표시

  // 라벨 박스 그리기
  rectangle(label, color, -1);

  // --- Text --------------------------------------------------------------------
  // 텍스트 위치 설정
  Point text_point(label.x + TEXT_X_MARGIN, label.y);

  // 텍스트 색상 설정 (배경색에 따라 텍스트 색상을 변경)
  const auto luminance  = (color.r * 0.299 + color.g * 0.587 + color.b * 0.114);
  auto       text_color = luminance > 127 ? Scalar(0, 0, 0, color.a) : Scalar(255, 255, 255, color.a);

  // 텍스트 그리기
  text(str, text_point, text_color, TEXT_FONT_SIZE);
}

void PangoDrawable::osdLabel(CString& str, const Rect2f& rect, const Scalar& color)
{
  osdLabel(str, rect * source_size_, color);
}

// --- Properties ----------------------------------------------------------------
void PangoDrawable::setFontFamily(CString& font_path)
{
  font_path_ = font_path;

  if (font_desc_)
    pango_font_description_free(font_desc_);

  font_desc_ = pango_font_description_from_string(font_path_.c_str());
}

void PangoDrawable::setFontSizeRelative(bool relative_width)
{
  relative_font_size_ = relative_width;
}

int PangoDrawable::pangoFontSize(const float font_scale, const bool relative_width /* = true */) const
{
  if (relative_width)
    return BASE_FONT_SIZE * PANGO_SCALE * (static_cast<float>(source_size_.width) / TEXT_SCALE_BASE * font_scale);
  else
    return BASE_FONT_SIZE * PANGO_SCALE * font_scale;
}

void PangoDrawable::setCairoColor(const Scalar& _color)
{
  auto color = _color.toRatio();

  if (source_channel_ == 3)
    cairo_set_source_rgb(cr_, color.r, color.g, color.b);
  else
    cairo_set_source_rgba(cr_, color.r, color.g, color.b, color.a);
}

void PangoDrawable::reflectToSource()
{
  if (prcess_image_.empty())
    return;

  if (source_channel_ != 3)
    return;

  int  pixels = input_source_.rows * input_source_.cols;
  auto src    = prcess_image_.ptr<uchar>();
  auto dst    = input_source_.ptr<uchar>();

  for (int i = 0; i < pixels; ++i)
  {
    dst[0] = src[0];  // B
    dst[1] = src[1];  // G
    dst[2] = src[2];  // R

    src += 4;
    dst += 3;
  }
}

#endif  // RS_VISION_WITH_PANGO

};  // namespace vision
};  // namespace rs
