#include <rowen/geometry.hpp>

namespace rs {
namespace vision {

// get vector of points
std::vector<Point2f> Polygon::vector() const
{
  std::vector<Point2f> points;
  for (int i = 0; i < n; i++)
    points.push_back(pt[i]);
  return points;
}

// -------------------------------------------------------------------------------
void Polygon::clear()
{
  n = 0;
}

void Polygon::add(const float& x, const float& y)
{
  if (n < MAX_POINT_POLYGON)
  {
    pt[n].x = x;
    pt[n].y = y;
    n++;
  }
}

void Polygon::add(const Point2f& p)
{
  add(p.x, p.y);
}

void Polygon::push_back(const Point2f& p)
{
  add(p);
}

int Polygon::size() const
{
  return n;
}

Point2f Polygon::center() const
{
  Point2f center;
  center.x = 0;
  center.y = 0;
  for (int i = 0; i < n; i++)
  {
    center.x += pt[i].x;
    center.y += pt[i].y;
  }
  center.x /= n;
  center.y /= n;
  return center;
}

const float Polygon::area() const
{
  float area0 = 0.f;
  for (int i = 0; i < n; i++)
  {
    int j = (i + 1) % n;
    area0 += pt[i].x * pt[j].y;
    area0 -= pt[i].y * pt[j].x;
  }
  return 0.5f * fabs(area0);
}

bool Polygon::pointInPolygon(Point2f p) const
{
  if (method_ppi == geometry::PPI_CUSTOM)
    return geometry::method::poly::pointInPolygon_custom(p, pt, n);
  else if (method_ppi == geometry::PPI_LASCHA)
    return geometry::method::poly::pointInPolygon_lascha(p, pt, n);
  else
    return false;
}

std::optional<float> Polygon::intersection(const Polygon& other, Polygon* inter_area)
{
  float iou = 0.f;
  if (intersection(other, iou, inter_area))
    return iou;
  else
    return std::nullopt;
}

bool Polygon::intersection(const Polygon& other, float& iou, Polygon* inter_area)
{
  bool intersect = false;

  Polygon inter;

  if (method_iou == geometry::IPA_GENERAL)
  {
    intersectionGeneral(other, iou, &inter);
  }
  else if (method_iou == geometry::IPA_SHPC)
  {
    intersectionSHPC(other, iou, &inter);
  }
  else
  {
    return false;
  }

  if (inter_area)
  {
    *inter_area = inter;
  }

  auto area_inter = inter.area();
  auto area_other = other.area();

  if (area_inter <= 0.f)
  {
    iou = 0.f;
    return false;
  }
  else
  {
    iou = area_inter / area_other * 100.f;
    return true;
  }

  return false;
}

bool Polygon::intersection(const Rect2f& rect, float& iou, Polygon* inter_area)
{
  return intersection(Polygon(rect), iou, inter_area);
}

void Polygon::intersectionGeneral(const Polygon& other, float& iou, Polygon* inter_area)
{
  for (int i = 0; i < n; i++)
  {
    if (other.pointInPolygon(pt[i]))
    {
      inter_area->add(pt[i]);
    }
  }

  for (int i = 0; i < other.size(); i++)
  {
    if (this->pointInPolygon(other[i]))
    {
      inter_area->add(other[i]);
    }
  }

  for (int i = 0; i < n; i++)
  {
    Point2f p[4];
    p[0] = pt[i];
    p[1] = pt[(i + 1) % n];
    for (int j = 0; j < other.size(); j++)
    {
      p[2] = other[j];
      p[3] = other[(j + 1) % other.size()];
      Point2f pinter;
      if (geometry::method::line::intersection(p[0], p[1], p[2], p[3], &pinter))
      {
        inter_area->add(pinter);
      }
    }
  }
  inter_area->sort_point();
}

void Polygon::intersectionSHPC(const Polygon& other, float& iou, Polygon* inter_area)
{
  int      i;
  Polygon  P1, P2;
  Polygon* p1 = &P1;
  Polygon* p2 = &P2;
  Polygon* tmp;

  int dir = shpc_poly_winding(other);
  shpc_poly_edge_clip(this, other.pt + other.n - 1, other.pt, dir, p2);
  for (i = 0; i < other.n - 1; i++)
  {
    tmp = p2;
    p2  = p1;
    p1  = tmp;
    if (p1->n == 0)
    {
      p2->n = 0;
      break;
    }
    shpc_poly_edge_clip(p1, other.pt + i, other.pt + i + 1, dir, p2);
  }

  inter_area->clear();
  for (i = 0; i < p2->n; i++) inter_area->add(p2->pt[i]);
}

//------------------------------------------------------------------------------------------------------------
//             IOU :  Intersection-over-Union
//------------------------------------------------------------------------------------------------------------

int Polygon::comp_point_with_angle(const void* a, const void* b)
{
  if (((PointAngle*)a)->angle < ((PointAngle*)b)->angle)
    return -1;
  else if (((PointAngle*)a)->angle > ((PointAngle*)b)->angle)
    return 1;
  else  // if ( ((PointAngle*)a)->angle == ((PointAngle*)b)->angle ) return 0;
    return 0;
}

void Polygon::sort_point()
{
  if (n <= 0)
    return;
  Point2f    ct = center();
  PointAngle pc[MAX_POINT_POLYGON];
  for (int i = 0; i < n; i++)
  {
    pc[i].p.x = pt[i].x;
    pc[i].p.y = pt[i].y;
    pc[i].angle =
        atan2f((float)(pt[i].y - ct.y), (float)(pt[i].x - ct.x));
  }
  qsort(pc, n, sizeof(PointAngle), Polygon::comp_point_with_angle);
  for (int i = 0; i < n; i++)
  {
    pt[i].x = pc[i].p.x;
    pt[i].y = pc[i].p.y;
  }
}

//------------------------------------------------------------------------------------------------------------
//             SHPC :  Sutherland-Hodgeman-Polygon-Clipping Algorihtm
//------------------------------------------------------------------------------------------------------------

static int cross(const Point2f* a, const Point2f* b)
{
  return a->x * b->y - a->y * b->x;
}

static Point2f* vsub(const Point2f* a, const Point2f* b, Point2f* res)
{
  res->x = a->x - b->x;
  res->y = a->y - b->y;
  return res;
}

static int line_sect(const Point2f* x0, const Point2f* x1,
                     const Point2f* y0, const Point2f* y1, Point2f* res)
{
  Point2f dx, dy, d;
  vsub(x1, x0, &dx);
  vsub(y1, y0, &dy);
  vsub(x0, y0, &d);
  float dyx = (float)cross(&dy, &dx);
  if (!dyx)
    return 0;
  dyx = cross(&d, &dx) / dyx;
  if (dyx <= 0 || dyx >= 1)
    return 0;
  res->x = int(y0->x + dyx * dy.x);
  res->y = int(y0->y + dyx * dy.y);
  return 1;
}

static int left_of(const Point2f* a, const Point2f* b, const Point2f* c)
{
  Point2f tmp1, tmp2;
  int     x;
  vsub(b, a, &tmp1);
  vsub(c, b, &tmp2);
  x = cross(&tmp1, &tmp2);
  return x < 0 ? -1 : x > 0;
}

void Polygon::shpc_poly_edge_clip(const Polygon* sub, const Point2f* x0, const Point2f* x1, int left, Polygon* res)
{
  int            i, side0, side1;
  Point2f        tmp;
  const Point2f* v0 = sub->pt + sub->n - 1;
  const Point2f* v1;
  res->clear();

  side0 = left_of(x0, x1, v0);
  if (side0 != -left)
    res->add(*v0);

  for (i = 0; i < sub->n; i++)
  {
    v1    = sub->pt + i;
    side1 = left_of(x0, x1, v1);
    if (side0 + side1 == 0 && side0)
    /* last point and current straddle the edge */
#if 0  // Error
      if (method::line::intersection(*x0, *x1, *v0, *v1, &tmp))
#else
      if (line_sect(x0, x1, v0, v1, &tmp))
#endif
      {
        res->add(tmp);
      }
    if (i == sub->n - 1)
      break;
    if (side1 != -left)
      res->add(*v1);
    v0    = v1;
    side0 = side1;
  }
}

int Polygon::shpc_poly_winding(const Polygon& p)
{
  return left_of(p.pt, p.pt + 1, p.pt + 2);
}

}  // namespace vision
}  // namespace rs
