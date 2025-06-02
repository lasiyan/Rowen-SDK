#include <rowen/utils/twilight.hpp>

int main()
{
  // 한국 (대전)
  rs::utils::Twilight::initialize();

  // 샌프란시스코
  // rs::utils::Twilight::initialize(37.7272, -123.032, -7);

  printf("Sun Rise : %ld\n", rs::utils::Twilight::sunrise());
  printf("Sun Raise : %s\n", rs::utils::Twilight::sunriseString().c_str());

  printf("Sun Set  : %ld\n", rs::utils::Twilight::sunset());
  printf("Sun Set   : %s\n", rs::utils::Twilight::sunsetString().c_str());

  printf("isDaylight : %s\n", rs::utils::Twilight::isDaylight() ? "Day" : "Night");

  return 0;
}
