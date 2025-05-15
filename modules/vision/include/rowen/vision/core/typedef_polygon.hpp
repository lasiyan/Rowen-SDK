#pragma once

#include <optional>
#include <vector>

#include "rowen/geometry/common.hpp"
#include "typedef_point.hpp"
#include "typedef_rect.hpp"

namespace rs {
namespace vision {

/**
 * @brief Polygon class. default type : float
 */
class Polygon
{
  constexpr static int MAX_POINT_POLYGON = 64;

  struct PointAngle
  {
    Point2f p;
    float   angle;
  };

 public:
  Polygon(int n_ = 0)
  {
    assert(n_ >= 0 && n_ < MAX_POINT_POLYGON);
    n = n_;
  }

  Polygon(const std::vector<Point2f>& ptfs)
  {
    for (const auto& p : ptfs)
    {
      this->add(p.x, p.y);
      if (n == MAX_POINT_POLYGON)
        break;
    }
  }

  Polygon(const Rect2f& rect)
  {
    this->add(rect.x, rect.y);
    this->add(rect.x + rect.width, rect.y);
    this->add(rect.x + rect.width, rect.y + rect.height);
    this->add(rect.x, rect.y + rect.height);
  }

  const Point2f& operator[](int index) const
  {
    assert(index >= 0 && index < n);
    return pt[index];
  }

  Point2f& operator[](int index)
  {
    assert(index >= 0 && index < n);
    return pt[index];
  }

  bool operator==(const Polygon& p) const
  {
    if (n != p.n)
      return false;
    for (int i = 0; i < n; i++)
    {
      if (pt[i] != p.pt[i])
        return false;
    }
    return true;
  }

  bool operator!=(const Polygon& p) const
  {
    return !(*this == p);
  }

  void operator<<(const Point2f& p)
  {
    add(p);
  }

  // set algorithm
  void usingSHPC() { method_iou = geometry::IPA_SHPC; }
  void usingRexGenPPI()
  {
    method_iou = geometry::IPA_GENERAL;
    method_ppi = geometry::PPI_CUSTOM;
  }
  void usingLaschaPPI()
  {
    method_iou = geometry::IPA_GENERAL;
    method_ppi = geometry::PPI_LASCHA;
  }

  // get vector of points
  std::vector<Point2f> vector() const;

  // -----------------------------------------------------------------------------

  void clear();

  void add(const float& x, const float& y);

  void add(const Point2f& p);

  void push_back(const Point2f& p);

  int size() const;

  Point2f center() const;

  const float area() const;

  bool pointInPolygon(Point2f p) const;

  std::optional<float> intersection(const Polygon& other, Polygon* inter_area = nullptr);

  bool intersection(const Polygon& other, float& iou, Polygon* inter_area = nullptr);
  bool intersection(const Rect2f& rect, float& iou, Polygon* inter_area = nullptr);

 private:
  void intersectionGeneral(const Polygon& other, float& iou, Polygon* inter_area);
  void intersectionSHPC(const Polygon& other, float& iou, Polygon* inter_area);

  // for General
  static int comp_point_with_angle(const void* a, const void* b);
  void       sort_point();

  // for SHPC
  void shpc_poly_edge_clip(const Polygon* sub, const Point2f* x0, const Point2f* x1, int left, Polygon* res);
  int  shpc_poly_winding(const Polygon& p);
  // void       shpc_intersectPolygonSHPC(const Polygon& sub, const Polygon& clip, Polygon& res);

 public:
  Point2f pt[MAX_POINT_POLYGON] = {};
  int     n                     = 0;
  int     method_ppi            = geometry::PPI_CUSTOM;
  int     method_iou            = geometry::IPA_GENERAL;
};

};  // namespace vision
};  // namespace rs
