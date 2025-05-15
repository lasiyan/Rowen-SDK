#include "rowen/core/function.hpp"

namespace rs {

bool containNonASCII(const std::string& str)
{
  for (size_t i = 0; i < str.size();)
  {
    unsigned char c = static_cast<unsigned char>(str[i]);

    if ((c & 0x80) == 0)  // 1바이트 문자 (영문, 숫자, 특수문자 등)
      i += 1;
    else
      return true;
  }

  return false;  // 전체가 ASCII 문자면 false 반환
}

}  // namespace rs
