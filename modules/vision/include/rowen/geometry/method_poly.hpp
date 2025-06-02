#pragma once

#include <rowen/vision/type_traits.hpp>

namespace rs {
namespace geometry {
namespace method {
namespace poly {

// static bool pointInPolygon_custom(rs::Point p, const rs::Point* points, int n)
// {
//   int  i, j;
//   bool c = false;
//   for (i = 0, j = n - 1; i < n; j = i++)
//   {
//     if (((points[i].y >= p.y) != (points[j].y >= p.y)) &&
//         (p.x <= (points[j].x - points[i].x) * (p.y - points[i].y) /
//                         (points[j].y - points[i].y) +
//                     points[i].x))
//       c = !c;
//   }
//   return c;
// }
static bool pointInPolygon_custom(rs::Point2f p, const rs::Point2f* points, int n)
{
  int  i, j;
  bool c = false;
  for (i = 0, j = n - 1; i < n; j = i++)
  {
    if (((points[i].y >= p.y) != (points[j].y >= p.y)) &&
        (p.x <= (points[j].x - points[i].x) * (p.y - points[i].y) /
                        (points[j].y - points[i].y) +
                    points[i].x))
      c = !c;
  }
  return c;
}

// static bool pointInPolygon_lascha(rs::Point p, const rs::Point* points, int corners)
// {
//   int  i, j = corners - 1;
//   bool oddNodes = false;

//   for (i = 0; i < corners; i++)
//   {
//     if ((points[i].y < p.y && points[j].y >= p.y ||
//          points[j].y < p.y && points[i].y >= p.y) &&
//         (points[i].x <= p.x || points[j].x <= p.x))
//     {
//       oddNodes ^=
//           (points[i].x + (p.y - points[i].y) / (points[j].y - points[i].y) *
//                              (points[j].x - points[i].x) <
//            p.x);
//     }
//     j = i;
//   }
//   return oddNodes;
// }
static bool pointInPolygon_lascha(rs::Point2f p, const rs::Point2f* points, int corners)
{
  int  i, j = corners - 1;
  bool oddNodes = false;

  for (i = 0; i < corners; i++)
  {
    if ((points[i].y < p.y && points[j].y >= p.y ||
         points[j].y < p.y && points[i].y >= p.y) &&
        (points[i].x <= p.x || points[j].x <= p.x))
    {
      oddNodes ^=
          (points[i].x + (p.y - points[i].y) / (points[j].y - points[i].y) *
                             (points[j].x - points[i].x) <
           p.x);
    }
    j = i;
  }
  return oddNodes;
}

};  // namespace poly
};  // namespace method
};  // namespace geometry
};  // namespace rs
