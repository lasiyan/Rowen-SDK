#pragma once

#include <rowen/core/format.hpp>
#include <string>
#include <vector>

namespace rs {

class Table
{
 public:
  enum HeaderAlign
  {
    ALIGN_LEFT,
    ALIGN_CENTER,
    ALIGN_RIGHT,
  };

  struct Header
  {
    std::string    name;
    mutable size_t width;
    HeaderAlign    align = ALIGN_LEFT;
  };
  using HeaderList = std::vector<Header>;

  class Cell
  {
   public:
    std::string value;

    template <typename T>
    Cell(const T& v) : value(toString(v))
    {
    }

   private:
    template <typename T>
    static std::string toString(const T& v)
    {
      using TYPE = std::decay_t<T>;

      if constexpr (std::is_same_v<TYPE, bool>)
        return v ? "true" : "false";
      else if constexpr (std::is_floating_point_v<TYPE>)
        return rs::format("%.3f", v);
      else if constexpr (std::is_integral_v<TYPE>)
        return std::to_string(v);
      else if constexpr (std::is_enum_v<TYPE>)
        return std::to_string(static_cast<std::underlying_type_t<TYPE>>(v));
      else if constexpr (std::is_convertible_v<TYPE, std::string>)
        return std::string(v);
      else if constexpr (std::is_pointer_v<TYPE>)
        return v ? rs::format("0x%p", v) : "nullptr";
      else
        return std::string("UNSUPPORTED");
    }
  };
  using Content     = std::vector<Cell>;
  using ContentList = std::vector<Content>;

 public:
  // 테이블 구성
  void setTitle(const std::string& title);
  void setHeader(const Header& header);
  void setHeaders(const HeaderList& headers);
  void setCtHeader(const Header& header);
  void setCtHeaders(const HeaderList& headers);
  void setContent(const Content& content);
  void setContents(const ContentList& contents);

  // 옵션 설정
  void optionShowHeader(bool set);
  void optionRowDivider(bool set);

  // 출력 함수
  std::string          string() const;
  const char*          c_str() const;
  friend std::ostream& operator<<(std::ostream& os, const Table& table);

  // 확장 유틸리티
  void sortedByColumn(const size_t column, const bool ascending = true);             // 기본값 : 오름차순 정렬
  void sortedByColumn(const std::string& column_name, const bool ascending = true);  // 기본값 : 오름차순 정렬

 private:
  std::string generator() const;

  // Helper
  static std::string alignText(const std::string& str, const size_t width, const Table::HeaderAlign align);
  static std::string repeatChar(const std::string_view ch, const size_t count);
  static size_t      getDisplayWidth(const std::string& str);

 public:
  Table()                        = default;
  ~Table()                       = default;
  Table(const Table&)            = delete;
  Table& operator=(const Table&) = delete;
  Table(Table&&)                 = default;
  Table& operator=(Table&&)      = default;

 private:
  std::string title_;
  HeaderList  headers_;
  ContentList contents_;
  bool        option_show_header_ = true;
  bool        option_row_divider_ = false;

  mutable std::string cached_table_;  // for c_str()
};

};  // namespace rs
