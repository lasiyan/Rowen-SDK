#include <rowen/utils/table.hpp>
#include <sstream>
#include <string_view>

namespace rs {

std::string Table::generator() const
{
  constexpr int CELL_MARGIN = 2;
  const bool    has_title   = (title_.empty() == false);

  std::ostringstream oss;

  // Header 이름의 길이가 지정된 너비보다 길면, 너비를 늘린다.
  for (auto& h : headers_)
  {
    if (h.name.length() + CELL_MARGIN > h.width)
      h.width = h.name.length() + CELL_MARGIN;
  }

  // --- make 관련 람다 함수 모음 ---------------------------------------------------
  const auto MakeBorder = [&](std::string_view left, std::string_view mid, std::string_view right) {
    oss << left;
    for (size_t i = 0; i < headers_.size(); ++i)
    {
      oss << repeatChar("─", headers_[i].width + CELL_MARGIN);
      if (i == headers_.size() - 1)
      {
        oss << right;
      }
      else
      {
        oss << mid;
      }
    }
    oss << "\n";
  };

  const auto MakeRow = [&](const std::vector<std::string>& strings) {
    oss << "│";
    for (size_t i = 0; i < headers_.size(); ++i)
    {
      std::string str = i < strings.size() ? strings[i] : "";
      oss << " " << alignText(str, headers_[i].width, headers_[i].align) << " │";
    }
    oss << "\n";
  };

  const auto TotalWidth = [&]() {
    size_t total_width = 0;
    for (const auto& h : headers_)
      total_width += h.width + CELL_MARGIN;
    total_width += headers_.size() - 1;
    return total_width;
  };
  // -----------------------------------------------------------------------------

  // 테이블(표) 상단 경계선
  (has_title) ? MakeBorder("┌", "─", "┐") : MakeBorder("┌", "┬", "┐");  // ┌───────────┐  또는  ┌───┬───┬───┐

  // 타이틀이 있는 경우, 타이틀을 출력한다.
  if (has_title)
  {
    size_t total_width = TotalWidth();

    oss << "│" << alignText(title_, total_width, ALIGN_CENTER) << "│\n";
    MakeBorder("├", "┬", "┤");  // ├───────────┬───────────┬───────────┤
  }

  // 헤더 영역 출력이 선택된 경우 헤더를 출력한다.
  if (option_show_header_)
  {
    std::vector<std::string> targets;
    for (const auto& h : headers_)
      targets.push_back(h.name);

    MakeRow(targets);           // │   Header  │   Header   │  Header   │
    MakeBorder("├", "┼", "┤");  // ├───────────┼───────────┼───────────┤
  }

  // 내용 영역 출력
  for (size_t r = 0; r < contents_.size(); ++r)
  {
    std::vector<std::string> targets;
    for (size_t i = 0; i < headers_.size(); ++i)
      targets.push_back(i < contents_[r].size() ? contents_[r][i].value : "");

    MakeRow(targets);  // │  Value  │  Value  │  Value  │

    // 마지막 행이 아니고, 행 구분선이 선택된 경우에만 행 구분선을 출력한다.
    if (option_row_divider_ && (r != contents_.size() - 1))
    {
      MakeBorder("├", "┼", "┤");  // ├───────────┼───────────┼───────────┤
    }
  }

  // 테이블(표) 하단 경계선
  MakeBorder("└", "┴", "┘");  // └───────────┴───────────┴───────────┘

  return oss.str();
}

std::string Table::alignText(const std::string& str, const size_t width, const Table::HeaderAlign align)
{
  size_t len = str.length();
  if (len >= width)
    return str;

  size_t pad = width - len;

  switch (align)
  {
    case Table::ALIGN_RIGHT:  return std::string(pad, ' ') + str;
    case Table::ALIGN_CENTER: return std::string(pad / 2, ' ') + str + std::string(pad - pad / 2, ' ');
    case Table::ALIGN_LEFT:
    default:                  return str + std::string(pad, ' ');
  }
}

std::string Table::repeatChar(const std::string_view ch, const size_t count)
{
  std::string s;
  s.reserve(ch.size() * count);
  for (size_t i = 0; i < count; ++i)
    s.append(ch);
  return s;
}

// --- Configurer ----------------------------------------------------------------

void Table::setTitle(const std::string& title)
{
  title_ = title;
}

void Table::setHeader(const Header& header)
{
  headers_.push_back(header);
}

void Table::setHeaders(const HeaderList& headers)
{
  headers_ = headers;
}

void Table::setContent(const Content& content)
{
  contents_.push_back(content);
}

void Table::setContents(const ContentList& contents)
{
  contents_ = contents;
}

std::string Table::string() const
{
  return generator();
}

const char* Table::c_str() const
{
  cached_table_ = string();
  return cached_table_.c_str();
}

std::ostream& operator<<(std::ostream& os, const Table& table)
{
  return os << table.string();
}

void Table::optionShowHeader(bool set)
{
  option_show_header_ = set;
}

void Table::optionRowDivider(bool set)
{
  option_row_divider_ = set;
}

}  // namespace rs
