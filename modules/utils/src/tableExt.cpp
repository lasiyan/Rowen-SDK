#include <algorithm>
#include <rowen/utils/table.hpp>

namespace rs {

void Table::sortedByColumn(const size_t index, bool ascending)
{
  std::sort(contents_.begin(), contents_.end(), [index, ascending](const Content& a, const Content& b) {
    std::string_view av = (index < a.size()) ? std::string_view(a[index].value) : std::string_view("");
    std::string_view bv = (index < b.size()) ? std::string_view(b[index].value) : std::string_view("");
    return ascending ? av < bv : av > bv;
  });
}

void Table::sortedByColumn(const std::string& name, bool ascending)
{
  auto it = std::find_if(headers_.begin(), headers_.end(), [&](const Header& h) { return h.name == name; });
  if (it != headers_.end())
  {
    size_t index = std::distance(headers_.begin(), it);
    sortedByColumn(index, ascending);
  }
}

}  // namespace rs
