#pragma once

#include <filesystem>
#include <iostream>
#include <string>
#include <type_traits>

#include "rapidjson/document.h"
#include "rapidjson/error/en.h"         // IWYU pragma: export
#include "rapidjson/filereadstream.h"   // IWYU pragma: export
#include "rapidjson/filewritestream.h"  // IWYU pragma: export
#include "rapidjson/istreamwrapper.h"   // IWYU pragma: export
#include "rapidjson/ostreamwrapper.h"   // IWYU pragma: export
#include "rapidjson/prettywriter.h"     // IWYU pragma: export
#include "rapidjson/stringbuffer.h"     // IWYU pragma: export
#include "rapidjson/writer.h"           // IWYU pragma: export

#if defined(__GNUC__) || defined(__clang__)
  #include <cxxabi.h>
#endif

#ifndef RS_JSON_ACTIVATE_PREVIEW_SET
  #define RS_JSON_ACTIVATE_PREVIEW_SET false
#endif

#ifndef RS_JSON
  #define RS_JSON             rapidjson
  #define RS_JSON_HELPER      rapidjson::Helper
  #define RS_JSON_ROOT        rapidjson::Document
  #define RS_JSON_TYPE_NULL   rapidjson::kNullType
  #define RS_JSON_TYPE_OBJECT rapidjson::kObjectType
  #define RS_JSON_TYPE_ARRAY  rapidjson::kArrayType
#else
  #error "RS_JSON is already defined."
#endif

// Do not use this function directly.
template <typename T>
T __rs_rapidjson_type_traits_as__(const rapidjson::Value& root)
{
  using base_type = typename std::decay_t<T>;

  // clang-format off
  if constexpr      (std::is_same_v<base_type, char> ||
                     std::is_same_v<base_type, short> ||
                     std::is_same_v<base_type, int> ||
                     std::is_same_v<base_type, int8_t> ||
                     std::is_same_v<base_type, int16_t> ||
                     std::is_same_v<base_type, int32_t>)            return static_cast<T>(root.GetInt());
  else if constexpr (std::is_same_v<base_type, unsigned int> ||
                     std::is_same_v<base_type, uint8_t> ||
                     std::is_same_v<base_type, uint16_t> ||
                     std::is_same_v<base_type, uint32_t>)           return static_cast<T>(root.GetUint());
  else if constexpr (std::is_same_v<base_type, long> ||
                     std::is_same_v<base_type, int64_t>)            return static_cast<T>(root.GetInt64());
  else if constexpr (std::is_same_v<base_type, uint64_t>)           return root.GetUint64();
  else if constexpr (std::is_same_v<base_type, float>)              return root.GetFloat();
  else if constexpr (std::is_same_v<base_type, double>)             return root.GetDouble();
  else if constexpr (std::is_same_v<base_type, bool>)               return root.GetBool();
  else if constexpr (std::is_same_v<base_type, std::string>)        return root.GetString();
  else if constexpr (std::is_same_v<base_type, const char*>)        return root.GetString();
  else
    throw std::runtime_error("__rs_rapidjson_type_traits_as__ : Unsupported type");
  // clang-format on
}

template <typename First>
bool __rs_rapidjson_get_validation__(const rapidjson::Value& root, First first)
{
  if (root.IsNull())
    return false;

  if constexpr (std::is_integral_v<First>)
    return first < root.Size();
  else
    return root.HasMember(first);
}

#if RS_JSON_ACTIVATE_PREVIEW_SET
template <typename T, typename FirstKey, typename... Keys>
void __rs_rapidjson_set_impl__(const T& variable, rapidjson::Value& root, rapidjson::Document::AllocatorType& allocator, const FirstKey& first, const Keys&... args)
{
  if constexpr (std::is_integral_v<FirstKey>)
  {
  }
  else
  {
    if (root.HasMember(first) == false)
      root.AddMember(rapidjson::Value(first, allocator), rapidjson::Value(rapidjson::kObjectType), allocator);
    else
      root[first].SetObject();  // If it is not an object, it will be overwritten;
  }

  if constexpr (sizeof...(Keys) == 0)
  {
    if (root[first].IsArray())
      root[first].PushBack(rapidjson::Value(variable), allocator);
    else
      root[first] = rapidjson::Value(variable);
  }
  else
  {
    __rs_rapidjson_set_impl__(variable, root[first], allocator, args...);
  }
}
#endif

namespace rapidjson {

template <typename T, typename First, typename... Args>
bool get(T& variable, const rapidjson::Value& root, First first, Args... args)
{
  try
  {
    if (__rs_rapidjson_get_validation__(root, first) == false)
      return false;

    if constexpr (sizeof...(Args) == 0)
    {
      variable = __rs_rapidjson_type_traits_as__<T>(root[first]);
      return true;
    }
    else
    {
      return get(variable, root[first], args...);
    }
  }
  catch (const std::runtime_error& e)
  {
#if defined(__GNUC__) || defined(__clang__)
    auto tname = __cxxabiv1::__cxa_demangle(typeid(T).name(), nullptr, nullptr, nullptr);
#else
    auto tname = typeid(T).name();
#endif
    std::cerr << "rapidjson::get(" << tname << "&, " << first << ", ...) : " << e.what() << std::endl;
    return false;
  }
}

template <typename T, typename First, typename... Args>
T get(const rapidjson::Value& root, First first, Args... args)
{
  try
  {
    if (__rs_rapidjson_get_validation__(root, first) == false)
      return T{};

    if constexpr (sizeof...(Args) == 0)
      return __rs_rapidjson_type_traits_as__<T>(root[first]);
    else
      return get<T>(root[first], args...);
  }
  catch (const std::runtime_error& e)
  {
#if defined(__GNUC__) || defined(__clang__)
    auto tname = __cxxabiv1::__cxa_demangle(typeid(T).name(), nullptr, nullptr, nullptr);
#else
    auto tname = typeid(T).name();
#endif
    std::cerr << "rapidjson::get<" << tname << ">(" << first << ", ...) : " << e.what() << std::endl;

    return T{};
  }
}

template <typename First, typename... Args>
bool exist(const rapidjson::Value& root, First first, Args... args)
{
  try
  {
    if (__rs_rapidjson_get_validation__(root, first) == false)
      return false;

    if constexpr (sizeof...(Args) == 0)
      return true;
    else
      return exist(root[first], args...);
  }
  catch (std::runtime_error& e)
  {
    return false;
  }
}

#if RS_JSON_ACTIVATE_PREVIEW_SET
template <typename T, typename FirstKey, typename... Keys>
void set(const T& variable, Document& root, const FirstKey& first, const Keys&... args)
{
  if (root.IsObject() == false && root.IsNull())
    root.SetObject();
  else
    assert(root.IsObject());

  __rs_rapidjson_set_impl__(variable, root, root.GetAllocator(), first, args...);
}

template <typename T>
void setArray(const T& variable, Document& root, const std::string& key = "")
{
  assert(root.IsArray());

  if (key.empty() == false && root.HasMember(key.c_str()) == false)
    root.AddMember(rapidjson::Value(key.c_str(), root.GetAllocator()), rapidjson::Value(rapidjson::kArrayType), root.GetAllocator());

  if (key.empty() == false)
    root[key.c_str()].PushBack(rapidjson::Value(variable), root.GetAllocator());
  else
    root.PushBack(rapidjson::Value(variable), root.GetAllocator());
}
#endif

namespace Helper {

bool read(const std::filesystem::path& file, rapidjson::Document* root);

bool read(const uint8_t* buffer, const size_t size, rapidjson::Document* root);

bool write(const std::filesystem::path& file, const rapidjson::Document& root);

std::string string(const rapidjson::Document& root, bool pretty = false);

void print(const rapidjson::Document& root, const char* title = nullptr);

};  // namespace Helper
};  // namespace rapidjson
