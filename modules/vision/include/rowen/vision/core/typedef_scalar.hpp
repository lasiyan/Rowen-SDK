#pragma once

#include <cstdint>

namespace rs {
namespace vision {

template <typename T>
class Scalar_
{
 public:
  Scalar_() { b = g = r = a = 0; }
  Scalar_(T _b, T _g, T _r) : r(_r), g(_g), b(_b) { a = 255; }
  Scalar_(T _b, T _g, T _r, T _a) : r(_r), g(_g), b(_b), a(_a) {}
  Scalar_(const Scalar_<T>& c) : r(c.r), g(c.g), b(c.b), a(c.a) {}
  Scalar_(uint32_t HEX_RGBA)
  {
    bool rgba = (HEX_RGBA & 0xFF000000) != 0;
    r         = (HEX_RGBA >> (rgba ? 24 : 16)) & 0xFF;
    b         = (HEX_RGBA >> (rgba ? 16 : 8)) & 0xFF;
    g         = (HEX_RGBA >> (rgba ? 8 : 0)) & 0xFF;
    a         = rgba ? (HEX_RGBA >> 0) & 0xFF : 255;
  }

  operator uint32_t() const
  {
    return (static_cast<uint32_t>(r) << 24 | static_cast<uint32_t>(g) << 16 | static_cast<uint32_t>(b) << 8 | static_cast<uint32_t>(a));
  }

  operator int32_t() const
  {
    return (static_cast<int32_t>(r) << 16 | static_cast<int32_t>(g) << 8 | static_cast<int32_t>(b));
  }

  bool operator==(const Scalar_<T>& c) const { return r == c.r && g == c.g && b == c.b && a == c.a; }
  bool operator!=(const Scalar_<T>& c) const { return r != c.r || g != c.g || b != c.b || a != c.a; }

  Scalar_<T>& operator=(const Scalar_<T>& c)
  {
    r = c.r;
    g = c.g;
    b = c.b;
    a = c.a;
    return *this;
  }

 public:
  Scalar_<float> toRatio() const  // 0 ~ 1.0f
  {
    return { b / 255.0f, g / 255.0f, r / 255.0f, a / 255.0f };
  }

 public:
  T b = 0, g = 0, r = 0, a = 0;
};

using Scalar2i = Scalar_<int>;
using Scalar2f = Scalar_<float>;
using Scalar   = Scalar2f;

};  // namespace vision
};  // namespace rs
