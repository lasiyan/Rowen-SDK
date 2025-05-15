#include <cstdarg>
#include <iostream>

#include "rowen/core.hpp"
#include "rowen/core/time.hpp"

void printf2(const char* format, ...)
{
  va_list args;
  va_start(args, format);
  vprintf(format, args);
  va_end(args);
}

int main()
{
  // 테스트 타임스탬프 입력
  std::vector<std::string> test_timestamps = {
    "2024-12-01T12:32:45.321",        // ISO 8601
    "2024-12-01 12:32:45.123456789",  // 공백 구분 + 9자리 소수점
    "2024/12/01 12:32:45",            // 슬래시 구분, 밀리초 없음
    "01-12-2024 12:32:45.4567",       // 날짜-월-연도 + 4자리 소수점
    "20241201_123245.999999999",      // 날짜_시간 + 9자리 소수점
    "20241201 123245.12",             // 날짜 시간 + 2자리 소수점
    "202412011232",                   // 날짜시간 + 6자리 소수점
    "2024 12 01 12 32 45.777"
  };

  for (const auto& ts : test_timestamps)
  {
    try
    {
      auto nanoseconds = rs::time::from_string(ts);
      std::cout << "Input: " << ts << " -> Nanoseconds since epoch: " << nanoseconds << "ns" << std::endl;
    }
    catch (const std::exception& ex)
    {
      std::cerr << "Error parsing timestamp: " << ts << " -> " << ex.what() << std::endl;
    }
  }

  return 0;
}
