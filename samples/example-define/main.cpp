#include <rowen/core.hpp>

struct TestStruct
{
  int value1, value2;
  TestStruct(const int value1, const int value2) : value1(value1), value2(value2) {}
};

void test_return_if_void(const bool condition)
{
  return_if(condition);
  printf("test_return_if_void\n");
}

std::string test_return_if_string(const bool condition, const std::string& value)
{
  return_if(condition, value);
  return value;
}

TestStruct test_return_if_var2(const bool condition)
{
  return_if(condition, { 1, 2 });
  return { 3, 4 };
}

rs::response_t test_return_if_response(const bool condition)
{
  rs::response_t res;

  return_if(condition, res.set(rssProgressError, "response %s", "failed"));
  return res.set(rssSuccess, "response %s", "success");
}

int main()
{
  test_return_if_void(true);   // May not be printed
  test_return_if_void(false);  // Printed

  auto s1 = test_return_if_string(true, "failed");    // Error case
  auto s2 = test_return_if_string(false, "success");  // Success case
  printf("s1: %s\n", s1.c_str());
  printf("s2: %s\n", s2.c_str());

  auto t1 = test_return_if_var2(true);   // Error case
  auto t2 = test_return_if_var2(false);  // Success case
  printf("t1: %d, %d\n", t1.value1, t1.value2);
  printf("t2: %d, %d\n", t2.value1, t2.value2);

  auto r1 = test_return_if_response(true);   // Error case
  auto r2 = test_return_if_response(false);  // Success case
  printf("r1: %s\n", r1.c_str());
  printf("r2: %s\n", r2.c_str());

  return 0;
}
