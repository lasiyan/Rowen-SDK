// #include "drawing-test.hpp"
#include "rowen/core.hpp"
#include "rowen/vision.hpp"

constexpr auto alpha = 128;  // 0: 투명, 255: 불투명

int main(int argc, char** argv)
{
  auto img = rs::vision::io::image::read_cv(rs::get_execute_directory() / "test.jpg", cv::IMREAD_COLOR);
  rs::terminal(false, "rm -f res_pango.jpg");

  // --- 이미지 변환 --------------------------------------------------------------
  // 입력 이미지가 3채널인 경우 4채널로 변환 (속도 최적화를 위해 4채널 이미지 사용 권장)
  auto sc = rs::time::tick<microseconds>();
#if using_input_3channel_image
  //
#else
  rs::vision::converter::BGR2BGRA(img, img);
#endif
  auto s = rs::time::tick<microseconds>();
  printf("elapsed time: %lf ms (cvt)\n", (s - sc) / 1000.0);

  // --- PangoDrawable -----------------------------------------------------------
  // Drawable 객체 생성
  // * realtime_relfection 인자: 3채널의 경우 실시간 반영 여부 (복사 오버헤드가 크기 때문에 가급적 false 사용 권장)
  rs::vision::PangoDrawable drawable(img, false);

  // --- Drawing -----------------------------------------------------------------
  {
    std::string str = "Hello World!";

    // Point
    {
      rs::Point2f pt(0.5, 0.5);

      drawable.circle(pt, 10, { 0, 0, 255, alpha });
    }

    // Line
    {
      rs::Line l1(rs::Point(700, 100), rs::Point(1000, 200));

      drawable.line(l1, { 255, 255, 255, alpha }, 4);
    }

    // Rectangle
    {
      rs::Rect   r1(200, 200, 300, 300);
      rs::Rect2f r2(0.6, 0.2, 0.2, 0.2);

      drawable.rectangle(r1, { 0, 255, 0, alpha });

      drawable.rectangle(r2, { 0, 255, 0, alpha }, 5);
    }

    // Polylines
    {
      std::vector<rs::Point2f> p1 = {
        { 0.7, 0.7 },
        { 0.8, 0.8 },
        { 0.7, 0.9 }
      };

      drawable.polylines(p1, true, { 255, 255, 0, alpha });
    }

    // Font
    {
      rs::Point p1(300, 700);

      rs::Size tsize;
      drawable.text("Font Test Default", p1, { 0, 0, 0, alpha }, 0.9, &tsize);

      {
        int       base = 0;
        rs::Point p2(300, p1.y + tsize.height + base + 10);
        drawable.text("Font Test Large", p2, { 0, 0, 0, alpha }, 1.2);
      }
    }

    // setLabel
    {
      rs::Rect2f  r1(0.4, 0.6, 0.2, 0.2);
      std::string str1 = "test setLabel 1";
      // std::string str1 = "테스트 OSD 라벨 1";

      drawable.rectangle(r1, { 0, 255, 255, alpha });
      drawable.osdLabel(str1.c_str(), r1 * img.size(), { 0, 255, 255, alpha });

      rs::Rect    r2(300, -50, 150, 100);
      std::string str2 = "한글은 어때??";
      drawable.rectangle(r2, { 255, 0, 255, alpha }, -1);
      drawable.osdLabel(str2.c_str(), r2, { 255, 0, 255, alpha });
    }

    // setText (OSD)
    {
      int line = 10;

      std::string str1 = "test setText() 1";
      drawable.osdText(str1.c_str(), line, 1.2, alpha);

      std::string str2 = "한글 출력 테스트";
      drawable.osdText(str2.c_str(), line, 0.9, alpha);
    }

    auto e = rs::time::tick<microseconds>();
    printf("elapsed time: %lf ms\n", (e - s) / 1000.0);
  }

  // --- Image Save --------------------------------------------------------------
#if using_input_3channel_image
  // 3채널 이미지의 경우 input으로 사용했던 img에 그려진 결과가 반영되어 있지 않음.
  // 따라서, PangoDrawable 객체를 통해 그려진 결과를 img에 반영하기 위해 synchronize() 호출
  // synchronize() : 호출 이후 drawable 객체는 자원을 유지 (소멸자에서 자원 해제)
  // release() : 호출 이후 drawable 객체는 자원을 해제
  drawable.release();

  // release 이후 drawable 접근 시 예외(std::exception) 발생
  // --- drawable.circle(rs::Point2f(0.5, 0.5), 10, { 0, 0, 255, alpha });
#endif

  rs::vision::io::image::write(rs::get_execute_directory() / "res_pango.jpg", img);

  return 0;
}
