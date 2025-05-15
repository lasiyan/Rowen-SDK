#include "rowen/core/function.hpp"

namespace rs {

// PrettyFunction parser
static void removeLambda(std::string& pretty_func)
{
  std::string lambda_key("::<lambda()>");
  size_t      colons = pretty_func.find(lambda_key);
  if (colons != std::string::npos)
  {
    pretty_func.erase(colons, lambda_key.length());
  }
}

std::string pretty_func_class(const std::string& pretty_function)
{
  std::string pretty_func = pretty_function;
  removeLambda(pretty_func);

  // find method start point
  size_t method_start = pretty_func.find("(");
  if (method_start == std::string::npos)
    return pretty_func;
  else
    pretty_func = pretty_func.substr(0, method_start + 1);

  size_t colons = pretty_func.rfind("::");
  if (colons == std::string::npos)
    return "(NO CLASS)";
  else
  {  // include namespace
    size_t colons_prev, begin;
    if ((colons_prev = pretty_func.rfind("::", colons - 1)) ==
        std::string::npos)
      begin = pretty_func.rfind(" ", colons) + 1;
    else
      begin = colons_prev + 2;  // 2: length of "::"
    return pretty_func.substr(begin, colons - begin);
  }
}

std::string pretty_func_method(const std::string& pretty_function)
{
  std::string pretty_func = pretty_function;
  removeLambda(pretty_func);

  // find method start point
  size_t method_start = pretty_func.find("(");
  if (method_start == std::string::npos)
    return pretty_func;
  else
    pretty_func = pretty_func.substr(0, method_start + 1);

  size_t colons, begin;
  if ((colons = pretty_func.rfind("::")) == std::string::npos)
    begin = pretty_func.rfind(" ", colons) + 1;
  else
    begin = colons + 2;
  size_t end = pretty_func.rfind("(") - begin;
  return pretty_func.substr(begin, end) + "()";
}

}  // namespace rs
