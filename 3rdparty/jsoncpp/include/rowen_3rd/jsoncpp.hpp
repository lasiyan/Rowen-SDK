#pragma once

#include <rowen_3rd/jsoncpp/json.h>  // IWYU pragma: export

#include <filesystem>
#include <iostream>
#include <string>
#include <type_traits>

#if defined(__GNUC__) || defined(__clang__)
  #include <cxxabi.h>
#endif

// Do not use this function directly.
template <typename T>
T __rs_jsoncpp_type_traits_as__(const Json::Value& root)
{
  using base_type = typename std::decay_t<T>;

  // clang-format off
  if constexpr      (std::is_same_v<base_type, char> ||
                     std::is_same_v<base_type, short> ||
                     std::is_same_v<base_type, int> ||
                     std::is_same_v<base_type, int8_t> ||
                     std::is_same_v<base_type, int16_t> ||
                     std::is_same_v<base_type, int32_t>)            return static_cast<T>(root.asInt());
  else if constexpr (std::is_same_v<base_type, unsigned int> ||
                     std::is_same_v<base_type, uint8_t> ||
                     std::is_same_v<base_type, uint16_t> ||
                     std::is_same_v<base_type, uint32_t>)           return static_cast<T>(root.asUInt());
  else if constexpr (std::is_same_v<base_type, long> ||
                     std::is_same_v<base_type, int64_t>)            return static_cast<T>(root.asInt64());
  else if constexpr (std::is_same_v<base_type, uint64_t>)           return root.asUInt64();
  else if constexpr (std::is_same_v<base_type, float>)              return root.asFloat();
  else if constexpr (std::is_same_v<base_type, double>)             return root.asDouble();
  else if constexpr (std::is_same_v<base_type, bool>)               return root.asBool();
  else if constexpr (std::is_same_v<base_type, std::string>)        return root.asString();
  else if constexpr (std::is_same_v<base_type, const char*>)        return root.asCString();
  else if constexpr (std::is_same_v<base_type, Json::Value>)        return root;
  else
    throw Json::LogicError("__rs_jsoncpp_type_traits_as__ : Unsupported type");
  // clang-format on
}

template <typename First>
bool __rs_jsoncpp_get_validation__(const Json::Value& root, First first)
{
  if (root.empty() || root.isNull())
    return false;

  if constexpr (std::is_integral_v<First>)
    return first < root.size();
  else
    return root.isMember(first);
}

namespace Json {  // name of jsoncpp namespace
class Value;      // redeclare Json::Value (for. export)

template <typename T, typename First, typename... Args>
bool get(T& variable, const Json::Value& root, First first, Args... args)
{
  try
  {
    if (__rs_jsoncpp_get_validation__(root, first) == false)
      return false;

    if constexpr (sizeof...(Args) == 0)
    {
      variable = __rs_jsoncpp_type_traits_as__<T>(root[first]);
      return true;
    }
    else
    {
      return get(variable, root[first], args...);
    }
  }
  catch (const Json::LogicError& e)
  {
#if defined(__GNUC__) || defined(__clang__)
    auto tname = __cxxabiv1::__cxa_demangle(typeid(T).name(), nullptr, nullptr, nullptr);
#else
    auto tname = typeid(T).name();
#endif
    std::cerr << "Json::get(" << tname << "&, " << first << ", ...) : " << e.what() << std::endl;
    return false;
  }
}

template <typename T, typename First, typename... Args>
T get(const Json::Value& root, First first, Args... args)
{
  try
  {
    if (__rs_jsoncpp_get_validation__(root, first) == false)
      return T{};

    if constexpr (sizeof...(Args) == 0)
      return __rs_jsoncpp_type_traits_as__<T>(root[first]);
    else
      return get<T>(root[first], args...);
  }
  catch (const Json::LogicError& e)
  {
#if defined(__GNUC__) || defined(__clang__)
    auto tname = __cxxabiv1::__cxa_demangle(typeid(T).name(), nullptr, nullptr, nullptr);
#else
    auto tname = typeid(T).name();
#endif
    std::cerr << "Json::get<" << tname << ">(" << first << ", ...) : " << e.what() << std::endl;

    return T{};
  }
}

template <typename First, typename... Args>
bool exist(const Json::Value& root, First first, Args... args)
{
  try
  {
    if (__rs_jsoncpp_get_validation__(root, first) == false)
      return false;

    if constexpr (sizeof...(Args) == 0)
      return true;
    else
      return exist(root[first], args...);
  }
  catch (Json::LogicError& e)
  {
    return false;
  }
}

template <typename T>
bool set(const T& variable, Json::Value& root)
{
  return false;
}

template <typename T, typename First>
bool set(const T& variable, Json::Value& root, First first)
{
  root[first] = variable;
  return true;
}

template <typename T, typename First, typename Second, typename... Rest>
bool set(const T& variable, Json::Value& root, First&& first, Second&& second, Rest&&... rest)
{
  return set(variable, root[first], std::forward<Second>(second), std::forward<Rest>(rest)...);
}

#ifdef RS_JSON_ACTIVATE_PREVIEW_SET
template <typename T>
void set(const T& variable, Json::Value& root)
{
  if (root.isArray())
    root.append(variable);
  else
    root = variable;
}

template <typename T, typename First, typename... Args>
void set(const T& variable, Json::Value& root, First first, Args... args)
{
  if constexpr (sizeof...(Args) == 0)
    set(variable, root[first]);
  else
    set(variable, root[first], args...);
}
#endif

namespace Helper {

bool read(const std::filesystem::path& file, Json::Value* root);

bool read(const uint8_t* buffer, const size_t size, Json::Value* root);

bool write(const std::filesystem::path& file, const Json::Value& root);

std::string string(const Json::Value& root, bool pretty = false);

void print(const Json::Value& root, const char* title = nullptr);

};  // namespace Helper
};  // namespace Json
