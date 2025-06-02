#include <cstdio>
#include <memory>
#include <mutex>
#include <rowen/core/response.hpp>
#include <rowen_3rd/jsoncpp.hpp>  // IWYU pragma: keep

class Foo
{
 public:
  std::mutex mutex;
};

rs::response_json method_json(int a)
{
  rs::response_json res;

  switch (a)
  {
    case 1:
    {
      Json::Value value;
      value["key1"] = "value1";
      value["key2"] = "value2";
      res.content   = value;
      return res(1, "success", Json::Value(value));
    }
    case 2:
      return res(rssSuccess, "success");
    default:
      return rs::response_json(rssException, "exception");
  }
}

rs::response_t method_t()
{
  rs::response_void res;
  return res(rssNotFound, "method_t not found");
}

rs::response_void method_void()
{
  rs::response_t res;
  // return res(rssNotFound, "method_void not found");
  return res(rssNotFound, "method_void not found");
}

int main()
{
  // case 1. default constructor
  rs::response<int> response_int;
  printf("response_int : %d, %s\n", response_int.status, response_int.c_str());

  std::string       err_string1 = "error message 1";
  rs::response_void response_void(rssProgressError, err_string1);
  printf("response_void : %d, %s\n", response_void.status, response_void.c_str());

  rs::response_bool response_bool(false);
  printf("response_bool : %d, %s\n", response_bool.status, response_bool.c_str());

  // case 2. constructor with int vs bool
  rs::response_void response_void2(rssSuccess, "ignore error message");
  printf("response_void2 : %d, %s\n", response_void2.status, response_void2.c_str());

  rs::response_void response_void3(false);
  printf("response_void3 : %d, %s\n", response_void3.status, response_void3.c_str());

  const char*       err_cstring1 = "resource not found";
  rs::response_char response_char(rssNotFound, err_cstring1);
  printf("response_char : %d, %s\n", response_char.status, response_char.c_str());

  // case 3. specific constructor
  rs::response<Foo> response_foo(rssSuccess, "success");
  printf("response_foo : %d, %s\n", response_foo.status, response_foo.c_str());

  rs::response<Foo*> response_rfoo;
  printf("response_rfoo : %d, %s\n", response_rfoo.status, response_rfoo.c_str());

  rs::response<std::shared_ptr<Foo>> response_sfoo(321, "error");
  printf("response_sfoo : %d, %s\n", response_sfoo.status, response_sfoo.c_str());

  rs::response<std::unique_ptr<Foo>> response_ufoo(123, "error");
  printf("response_ufoo : %d, %s\n", response_ufoo.status, response_ufoo.c_str());

  // case 4. copy operator
  rs::response<int> response_int2;
  response_int2 = false;
  printf("response_int2 : %d, %s\n", response_int2.status, response_int2.c_str());

  response_int2 = rssProgressError;
  printf("response_int2 : %d, %s\n", response_int2.status, response_int2.c_str());

  // case 5. comparison operator
  // comparison operator : int
  if (response_int2 == rssSuccess)
    printf("response_int2 is success\n");
  else if (response_int2 == rssNotFound)
    printf("response_int2 is not found\n");
  else
    printf("response_int2 is not success\n");

  // comparison operator : bool
  if (response_int2 == true)
    printf("response_int2 is true\n");
  else
    printf("response_int2 is false\n");

  // comparison operator : bool
  if (response_int2)
    printf("response_int2 is true\n");
  else
    printf("response_int2 is false\n");

  printf("response_int2 : %s\n", response_int2 ? "true" : "false");

  // case 6. in function
  auto func1 = []() {
    rs::response<char> res;
    return res.set(123, "error message 123");
  };
  auto res = func1();
  printf("res : %d, %s\n", res.status, res.c_str());

  auto func2 = []() {
    std::shared_ptr<Foo>               foo = std::make_shared<Foo>();
    rs::response<std::shared_ptr<Foo>> res;
    res.content = foo;
    return res.set(456, "error message 456");
  };
  auto res2 = func2();
  printf("res2 : %d, %s, data: %p\n", res2.status, res2.c_str(), res2.content.get());

  // case 7. T from void
  rs::response_float response_float{ rssSuccess, "success", 3.14f };
  printf("response_float : %d, %s, %f\n", response_float.status, response_float.c_str(), response_float.content);

  auto func3 = []() {
    rs::response_void res;
    return res.set(rssProgressError, "server error");
  };
  auto temp      = func3();
  response_float = temp;
  printf("response_float : %d, %s, %f\n", response_float.status, response_float.c_str(), response_float.content);

  // Json
  rs::response_json response_json(rssSuccess, "success");
  printf("response_json : %d, %s\n", response_json.status, response_json.c_str());
  printf("response_json : %s\n", response_json.content.toStyledString().c_str());

  // response_t
  rs::response_t response_t(rssProgressError, "server is not running");
  printf("response_t : %d, %s\n", response_t.status, response_t.c_str());

  rs::response<double> response_double;
  response_double.content = 3.14;
  response_double         = response_t;
  printf("response_double : %d, %s\n", response_double.status, response_double.c_str());

  // case 8. method
  auto response_json1 = method_json(1);
  printf("response_json1 : %d, %s, %s\n", response_json1.status, response_json1.c_str(), response_json1.content.toStyledString().c_str());

  auto response_json2 = method_json(2);
  printf("response_json2 : %d, %s, %s\n", response_json2.status, response_json2.c_str(), response_json2.content.toStyledString().c_str());

  auto response_t1 = method_t();
  printf("response_t1 : %d, %s\n", response_t1.status, response_t1.c_str());

  auto response_void1 = method_void();
  printf("response_void1 : %d, %s\n", response_void1.status, response_void1.c_str());

  // same type operator=
  rs::response_int response_int3(rssConflict, "conflict", 1024);
  rs::response_int response_int4 = response_int3;
  printf("response_int4 : %d, %s, %d\n", response_int4.status, response_int4.c_str(), response_int4.content);

  rs::response_int response_int5;
  response_int5 = response_int4;
  printf("response_int5 : %d, %s, %d\n", response_int5.status, response_int5.c_str(), response_int5.content);

  rs::response_int response_int6(response_int5);
  printf("response_int6 : %d, %s, %d\n", response_int6.status, response_int6.c_str(), response_int6.content);

  // different type operator=
  rs::response_int response_int7;
  response_int7 = response_double;
  printf("response_int7 : %d, %s, %d\n", response_int7.status, response_int7.c_str(), response_int7.content);

  // string
#ifdef RS_RESPONSE_ALLOW_STRING_OPERATOR
  rs::response_json response_json3("test erro message");
  printf("response_json3 : %d, %s\n", response_json3.status, response_json3.c_str());

  response_json3 = "test error message 2";
  printf("response_json3 : %d, %s\n", response_json3.status, response_json3.c_str());
#endif

  // case 9. formatter (response_t `set` method only)
  auto response_t_format = [] {
    rs::response_t res;
    // return res.set(rssProgressError, "error message NO ARGS", 1);
    return res.set(rssProgressError, "error message %s %s %c %d", "Hello", "World", '!', 123);
  };
  auto res3 = response_t_format();
  printf("response_t_format   : %d, %s\n", res3.status, res3.c_str());

  auto response_int_format = [] {
    rs::response_int res;
    res.content = 456;
    return res.setFmt(rssProgressError, "error message %s %s %c %d", "Hello", "World", '!', 123);
  };
  auto res4 = response_int_format();
  printf("response_int_format : %d, %s, %d\n", res4.status, res4.c_str(), res4.content);

  return 0;
}
