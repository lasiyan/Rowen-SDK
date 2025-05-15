#include <iostream>

#include "rowen/core.hpp"
#include "rowen/geometry.hpp"

#ifdef TESTING_GEOMETRY_OPENCV
  #include "rowen/vision/drawable.hpp"
  #include "rowen/vision/io_image.hpp"
#endif

void test_line(rs::Mat* img_ptr = nullptr)
{
  rs::Size frame_size(1000, 1000);

#ifdef TESTING_GEOMETRY_OPENCV
  auto img   = img_ptr->clone();
  frame_size = img.size();
#endif

  std::cout << "------ start test_line ------" << std::endl;

  // 1. 점과 점 사이 거리
  rs::Point point1(300, 300);
  rs::Point point2(600, 600);

  auto dist = rs::geometry::method::line::distance(point1, point2);
  std::cout << "distance : " << dist << std::endl;
#ifdef TESTING_GEOMETRY_OPENCV
  {
    // drawing
    rs::vision::drawable::line(img, point1, point2, { 255, 0, 0 }, 2);
    rs::vision::drawable::line(img, point1, point2, { 255, 255, 0 }, 2);
    rs::vision::drawable::circle(img, point1, 5, { 0, 255, 0 }, 2);
    rs::vision::drawable::circle(img, point2, 5, { 0, 255, 0 }, 2);
  }
#endif

  // 2. 지정된 라인을 프레임 끝까지 연장
  rs::Line2f line1(0.225, 0.356, 0.5, 0.567);
  auto       line2 = rs::geometry::method::line::extension(line1, frame_size);

  rs::Line line3(100, 100, 500, 200);
  line3 = rs::geometry::method::line::extension(line3, frame_size);
#ifdef TESTING_GEOMETRY_OPENCV
  {
    // drawing
    rs::vision::drawable::line(img, line1, { 255, 0, 0 }, 2);
    rs::vision::drawable::circle(img, line1.p1, 5, { 0, 255, 0 }, 2);
    rs::vision::drawable::circle(img, line1.p2, 5, { 0, 255, 0 }, 2);
    rs::vision::drawable::line(img, line2, { 0, 0, 255 }, 2);

    rs::vision::drawable::line(img, line3, { 255, 0, 0 }, 2);
    rs::vision::drawable::circle(img, line3.p1, 5, { 0, 255, 0 }, 2);
    rs::vision::drawable::circle(img, line3.p2, 5, { 0, 255, 0 }, 2);
    rs::vision::drawable::line(img, line3, { 0, 0, 255 }, 2);
  }
#endif

  // 3. 두 라인의 교점
  rs::Line2f  comp1(0.5, 0.1, 0.3, 0.9);
  rs::Point2f res1;
  auto        res = rs::geometry::method::line::intersection(line1, comp1, &res1);
  std::cout << "intersection : " << res << std::endl;
#ifdef TESTING_GEOMETRY_OPENCV
  {
    // drawing
    rs::vision::drawable::line(img, line1, { 255, 0, 0 }, 2);
    rs::vision::drawable::line(img, comp1, { 0, 0, 255 }, 2);
    rs::vision::drawable::circle(img, res1, 5, { 0, 255, 0 }, 2);
  }
#endif

#ifdef TESTING_GEOMETRY_OPENCV
  rs::vision::io::image::write("result_line.jpg", img);
#endif
}

void test_poly(rs::Mat* img_ptr = nullptr)
{
#ifdef TESTING_GEOMETRY_OPENCV
  auto img = img_ptr->clone();
#endif

  std::cout << "------ start test_poly ------" << std::endl;
  auto start = rs::time::tick();

  rs::Polygon area;

#if 0  // Normal Polygon
  area << rs::Point(100, 100);
  area << rs::Point(200, 200);
  area << rs::Point(1400, 100);
  area << rs::Point(1500, 900);
  area << rs::Point(1000, 700);
  area << rs::Point(100, 900);
#else
  area << rs::Point(529, 63);
  area << rs::Point(650, 213);
  area << rs::Point(846, 217);
  area << rs::Point(756, 364);
  area << rs::Point(860, 512);
  area << rs::Point(652, 510);
  area << rs::Point(529, 662);
  area << rs::Point(407, 514);
  area << rs::Point(191, 510);
  area << rs::Point(302, 369);
  area << rs::Point(190, 213);
  area << rs::Point(408, 207);
#endif

#ifdef TESTING_GEOMETRY_OPENCV
  rs::vision::drawable::polylines(img, area.vector(), true, { 255, 0, 0 }, 4);
#endif

  rs::Polygon car;
  car << rs::Point(318, 111);
  car << rs::Point(850, 450);
  car << rs::Point(500, 600);
  car << rs::Point(10, 400);
#ifdef TESTING_GEOMETRY_OPENCV
  rs::vision::drawable::polylines(img, car.vector(), true, { 0, 0, 255 }, 4);
#endif

  rs::Polygon res;

  float iou = 0.0f;
  area.intersection(car, iou, &res);
  std::cout << "iou1 : " << iou << std::endl;

  auto result = area.intersection(car, &res);
  if (result.has_value())
  {
    std::cout << "iou2 : " << result.value() << std::endl;
  }

#ifdef TESTING_GEOMETRY_OPENCV
  rs::vision::drawable::polylines(img, res.vector(), true, { 0, 255, 255 }, 2);
#endif

  std::cout << "elapsed : " << rs::time::elapse_str(start) << std::endl;

#ifdef TESTING_GEOMETRY_OPENCV
  rs::vision::io::image::write("result_poly.jpg", img);
#endif
}

void test_shpc(rs::Mat* img_ptr = nullptr)
{
#ifdef TESTING_GEOMETRY_OPENCV
  auto img = img_ptr->clone();
#endif

  std::cout << "------ start test_shpc ------" << std::endl;

  auto tick = rs::time::tick();

  rs::Polygon area;
  area.usingSHPC();

#if 0  // Normal Polygon
  area << rs::Point(100, 100);
  area << rs::Point(200, 200);
  area << rs::Point(1400, 100);
  area << rs::Point(1500, 900);
  area << rs::Point(1000, 700);
  area << rs::Point(100, 900);
#else
  area << rs::Point(529, 63);
  area << rs::Point(650, 213);
  area << rs::Point(846, 217);
  area << rs::Point(756, 364);
  area << rs::Point(860, 512);
  area << rs::Point(652, 510);
  area << rs::Point(529, 662);
  area << rs::Point(407, 514);
  area << rs::Point(191, 510);
  area << rs::Point(302, 369);
  area << rs::Point(190, 213);
  area << rs::Point(408, 207);
#endif

#ifdef TESTING_GEOMETRY_OPENCV
  rs::vision::drawable::polylines(img, area.vector(), true, { 255, 0, 0 }, 4);
#endif

  rs::Polygon car;
  car << rs::Point(318, 111);
  car << rs::Point(850, 450);
  car << rs::Point(500, 600);
  car << rs::Point(10, 400);
#ifdef TESTING_GEOMETRY_OPENCV
  rs::vision::drawable::polylines(img, car.vector(), true, { 0, 0, 255 }, 4);
#endif

  rs::Polygon res;

  float iou = 0.0f;
  area.intersection(car, iou, &res);
  std::cout << "iou : " << iou << std::endl;

#ifdef TESTING_GEOMETRY_OPENCV
  rs::vision::drawable::polylines(img, res.vector(), true, { 0, 255, 255 }, 2);
#endif

  std::cout << "elapsed : " << rs::time::tick() - tick << " ns" << std::endl;

#ifdef TESTING_GEOMETRY_OPENCV
  rs::vision::io::image::write("result_shpc.jpg", img);
#endif
}

int main()
{
#ifdef TESTING_GEOMETRY_OPENCV
  auto img = rs::vision::io::image::read_cv("test.jpg");

  auto line_img = img.clone();
  test_line(&img);

  auto shpc_img = img.clone();
  test_shpc(&img);

  auto poly_img = img.clone();
  test_poly(&img);
#else
  test_line();
  test_shpc();
  test_poly();
#endif

  return 0;
}
