#include <unistd.h>

#include <filesystem>
#include <rowen_3rd/jsoncpp.hpp>

#ifdef __linux
std::filesystem::path get_execute_directory()
{
  char buffer[1024];
  ::memset(buffer, 0, sizeof(buffer));
  auto res = ::readlink("/proc/self/exe", buffer, sizeof(buffer));
  return buffer;

  std::filesystem::path path = buffer;
  return path.parent_path();
}
#else

#endif

int main()
{
  Json::Value root;

  Json::Helper::read(get_execute_directory() / "test.json", &root);
  Json::Helper::print(root, "title");

  //------------------------------------------------------------------------------
  // int
  int int1 = Json::get<int>(root, "int1"), int2;
  Json::get(int2, root, "int2");
  printf("int1 = %d, int2 = %d / type: %s, %s\n", int1, int2, typeid(int1).name(), typeid(int2).name());

  //------------------------------------------------------------------------------
  // float
  float float1 = Json::get<float>(root, "float1"), float2;
  Json::get(float2, root, "float2");
  printf("float1 = %f, float2 = %f / type: %s, %s\n", float1, float2, typeid(float1).name(), typeid(float2).name());

  //------------------------------------------------------------------------------
  // double
  double double1 = Json::get<double>(root, "double1"), double2;
  Json::get(double2, root, "double2");
  printf("double1 = %f, double2 = %f / type: %s, %s\n", double1, double2, typeid(double1).name(), typeid(double2).name());

  //------------------------------------------------------------------------------
  // bool
  bool bool1 = Json::get<bool>(root, "bool1"), bool2;
  Json::get(bool2, root, "bool2");
  printf("bool1 = %d, bool2 = %d / type: %s, %s\n", bool1, bool2, typeid(bool1).name(), typeid(bool2).name());

  //------------------------------------------------------------------------------
  // string
  std::string string1 = Json::get<std::string>(root, "str1"), string2;
  Json::get(string2, root, "str2");
  printf("string1 = %s, string2 = %s / type: %s, %s\n", string1.c_str(), string2.c_str(), typeid(string1).name(), typeid(string2).name());

  //------------------------------------------------------------------------------
  // const char*
  const char *c_string1 = Json::get<const char*>(root, "str1"), *c_string2;
  Json::get(c_string2, root, "str2");
  printf("c_string1 = %s, c_string2 = %s / type: %s, %s\n", c_string1, c_string2, typeid(c_string1).name(), typeid(c_string2).name());

  //------------------------------------------------------------------------------
  // auto type
  auto auto1 = Json::get<std::string>(root, "str1");
  printf("auto1 = %s / type: %s\n", auto1.c_str(), typeid(auto1).name());
  auto auto2 = Json::get<const char*>(root, "str1");
  printf("auto2 = %s / type: %s\n", auto2, typeid(auto2).name());

  //------------------------------------------------------------------------------
  // Invalid type
  char char1 = Json::get<char>(root, "int1"), char2 = '1';
  Json::get(char2, root, "int2");
  printf("char1 = %d, t2 = %d / type: %s, %s\n", char1, char2, typeid(char1).name(), typeid(char2).name());

  return 0;
}
