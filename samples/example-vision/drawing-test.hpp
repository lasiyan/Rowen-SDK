// Standard c++ library
#include <iostream>

// SDK
#include "rowen/core.hpp"
#include "rowen/vision.hpp"

constexpr auto alpha = 128;  // 0: 투명, 255: 불투명

static void drawing(const rs::Mat& img)
{
  rs::terminal(false, "rm -f res_cuda.jpg");

  std::string str = "Hello World!";

#if defined(RSDK_WITH_JETSON_UTILS)
  rs::GpuFrame gpu_frame;
  gpu_frame     = frame;
  gpu_frame.img = img;

  const auto& gpu_img = gpu_frame.img;
#endif

  // -- Point --------------------------------------------------------------------
  {
    rs::Point2f pt(0.5, 0.5);

    rs::vision::drawable::circle(img, pt, 10, { 0, 0, 255, alpha });
#if defined(RSDK_WITH_JETSON_UTILS)
    rs::vision::drawable::circle(gpu_img, pt, 10, { 0, 0, 255, alpha });
#endif
  }

  // -- Line ---------------------------------------------------------------------
  {
    rs::Line l1(rs::Point(700, 100), rs::Point(1000, 200));

    rs::vision::drawable::line(img, l1, { 255, 255, 255, alpha }, 4);
#if defined(RSDK_WITH_JETSON_UTILS)
    rs::vision::drawable::line(gpu_img, l1, { 255, 255, 255, alpha }, 4);
#endif
  }

  // -- Rectangle ----------------------------------------------------------------
  {
    rs::Rect   r1(200, 200, 300, 300);
    rs::Rect2f r2(0.6, 0.2, 0.2, 0.2);

    rs::vision::drawable::rectangle(img, r1, { 0, 255, 0, alpha });
#if defined(RSDK_WITH_JETSON_UTILS)
    rs::vision::drawable::rectangle(gpu_img, r1, { 0, 255, 0, alpha });
#endif

    rs::vision::drawable::rectangle(img, r2, { 0, 255, 0, alpha }, 5);
#if defined(RSDK_WITH_JETSON_UTILS)
    rs::vision::drawable::rectangle(gpu_img, r2, { 0, 255, 0, alpha }, 3);
#endif
  }

  // -- polylines ----------------------------------------------------------------
  {
    std::vector<rs::Point2f> p1 = {
      { 0.7, 0.7 },
      { 0.8, 0.8 },
      { 0.7, 0.9 }
    };

    rs::vision::drawable::polylines(img, p1, true, { 255, 255, 0, alpha });
#if defined(RSDK_WITH_JETSON_UTILS)
    rs::vision::drawable::polylines(gpu_img, p1, true, { 255, 255, 0, alpha });
#endif
  }

  // -- Font ---------------------------------------------------------------------
  {
    rs::Point p1(300, 700);

    rs::vision::drawable::text(img, "Font Test Default", p1, { 0, 0, 0 });
#if defined(RSDK_WITH_JETSON_UTILS)
    rs::vision::drawable::text(gpu_img, "Font Test Default", p1, { 0, 0, 0 });
#endif

    {
      int       base  = 0;
      auto      cv_sz = rs::vision::drawable::getTextSize("Font Test Default", &base);
      rs::Point p2(300, p1.y + cv_sz.height + base + 10);
      rs::vision::drawable::text(img, "Font Test Large", p2, { 0, 0, 0 }, rs::vision::drawable::LARGE_FONT);
    }
#if defined(RSDK_WITH_JETSON_UTILS)
    {
      auto      cu_sz = rs::vision::drawable::getTextSize("Font Test Default", gpu_img.size());
      rs::Point p2(300, p1.y + cu_sz.height + 10);
      rs::vision::drawable::text(gpu_img, "Font Test Large", p2, { 0, 0, 0 }, rs::vision::drawable::LARGE_FONT);
    }
#endif
  }

  // -- setText ------------------------------------------------------------------
  {
    int line = 33;

    std::string str1 = "test setText() 1";
    rs::vision::drawable::osd::setText(img, str1.c_str(), line);
#if defined(RSDK_WITH_JETSON_UTILS)
    rs::vision::drawable::osd::setText(gpu_img, str1.c_str(), line);
#endif

    std::string str2 = "test setText() 2";
    rs::vision::drawable::osd::setText(img, str2.c_str(), line);
#if defined(RSDK_WITH_JETSON_UTILS)
    rs::vision::drawable::osd::setText(gpu_img, str2.c_str(), line);
#endif
  }

  // -- setLabel -----------------------------------------------------------------
  {
    rs::Rect2f  r1(0.4, 0.6, 0.2, 0.2);
    std::string str1 = "test setLabel 1";

    rs::vision::drawable::osd::setLabel(img, str1.c_str(), r1 * img.size(), { 0, 255, 255, alpha });
#if defined(RSDK_WITH_JETSON_UTILS)
    rs::vision::drawable::osd::setLabel(gpu_img, str1.c_str(), r1 * gpu_img.size(), { 0, 255, 255, alpha });
#endif

    rs::Rect    r2(300, -50, 150, 100);
    std::string str2 = "test setLabel 2";
    rs::vision::drawable::rectangle(img, r2, { 255, 0, 255, alpha }, -1);
    rs::vision::drawable::osd::setLabel(img, str2.c_str(), r2, { 255, 0, 255, alpha });
  }

  rs::vision::io::image::write("res_opencv.jpg", img);
#if defined(RSDK_WITH_JETSON_UTILS)
  rs::vision::io::image::write("res_cuda.jpg", gpu_img);
#endif
}

inline void foo1(cv::Point p)
{
  std::cout << "foo1: " << p.x << ", " << p.y << std::endl;
}

inline void foo2(cv::Point& p)
{
  std::cout << "foo2: " << p.x << ", " << p.y << std::endl;
}

inline void foo3(const cv::Point p)
{
  std::cout << "foo3: " << p.x << ", " << p.y << std::endl;
}

inline void foo4(const cv::Point& p)
{
  std::cout << "foo4: " << p.x << ", " << p.y << std::endl;
}

inline void foo5(std::vector<cv::Point> v)
{
  std::cout << "foo5: " << v.at(0) << std::endl;
}

inline void run()
{
  std::vector<std::string> name = {
    "test"
  };
  std::string path;

#if 1  // TYPE TEST
  std::vector<rs::Point> pts{
    { 1, 2 },
    { 2, 3 },
    { 3, 4 }
  };
  rs::Point p(1, 2);
  foo1(p);
  foo2(p);
  foo3(p);
  foo4(p);
  // foo5(std::vector<cv::Point>(pts));

  printf("---Size---------------------------------------------------------\n");

  rs::Size   iSize(100, 200);
  rs::Size2f fSize(100.1, 200.1);

  printf("---Point--------------------------------------------------------\n");

  rs::vision::Point   iPoint(10, 20);
  rs::vision::Point2f fPoint(10.1, 20.2);

  cv::Point   cvPoint(10, 20);
  cv::Point2f cvfPoint(10.1, 20.2);

  rs::vision::Point iPoint2 = fPoint * iSize;
  auto              iPoint3 = fPoint * iSize;
  auto              iPoint4 = static_cast<rs::Point>(iPoint3);
  auto              fPoint1 = fPoint * fSize;

  printf("iPoint2: %d, %d\n", iPoint2.x, iPoint2.y);
  printf("iPoint3: %d, %d\n", iPoint3.x, iPoint3.y);
  printf("iPoint4: %d, %d\n", iPoint4.x, iPoint4.y);
  printf("fPoint1: %.1f, %.1f\n", fPoint1.x, fPoint1.y);

  rs::Point2f f1 = rs::Point2f(10.1, 20.2);
  rs::Point2f f2 = rs::Point2f(30.3, 40.4);
  f1             = f2;
  printf("f1: %f, %f\n", f1.x, f1.y);

  printf("---Rect --------------------------------------------------------\n");
  rs::Rect2f rsfRect(0.5, 0.5, 0.2, 0.1);
  rs::Rect   rsRectT1 = rsfRect * iSize;

  // printf("rsRectT1: %.0f, %.0f, %.0f, %.0f\n", rsRectT1.x, rsRectT1.y, rsRectT1.width, rsRectT1.height);
  printf("rsRectT1: %d, %d, %d, %d\n", rsRectT1.x, rsRectT1.y, rsRectT1.width, rsRectT1.height);

  printf("---Line --------------------------------------------------------\n");
  rs::Line2f rsfLine(0.5, 0.5, 0.2, 0.1);
  rs::Line   rsLineT1 = rsfLine * iSize;

  printf("rsLineT1: %d, %d, %d, %d\n", rsLineT1.p1.x, rsLineT1.p1.y, rsLineT1.p2.x, rsLineT1.p2.y);

  rs::Line   l1 = rs::Line(rs::Point(10, 20), rs::Point(30, 40));
  rs::Line2f l2 = rs::Line2f(rs::Point2f(10.1, 20.2), rs::Point2f(30.3, 40.4));

  std::cout << "l1: " << l1.length() << std::endl;
  std::cout << "l2: " << l2.length() << std::endl;

  printf("---Mat ---------------------------------------------------------\n");
  rs::Mat m1(100, 200, CV_8UC3);
  auto    m2 = m1.clone();

  printf("m1: %d, %d, %d\n", m1.cols, m1.rows, m1.channels());
  printf("m2: %d, %d, %d\n", m2.cols, m2.rows, m2.channels());

  printf("---Scalar ------------------------------------------------------\n");
  rs::Scalar c1 = 0xFFFFFF;
  rs::Scalar c2 = 0x10203040;

  uint c3 = c1;
  printf("c3: %02X\n", c3);

  int c4 = c1;
  printf("c4: %02X\n", c4);
#endif

  for (const auto& i : name)
  {
    path = rs::format("%s.jpg", i.c_str());

    // read image
    auto img = rs::vision::io::image::read<rs::Mat>(path);

    drawing(img);
  }
}
