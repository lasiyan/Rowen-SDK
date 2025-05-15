#include <cassert>
#include <iostream>

#include "rowen/utils/table.hpp"

using namespace rs;

void test_basic_table()
{
  Table table;
  table.setTitle("Test Table");
  table.setHeaders({ { "Name", 8, Table::ALIGN_LEFT },
                     { "Age", 3, Table::ALIGN_RIGHT },
                     { "City", 10, Table::ALIGN_CENTER },
                     { "Float", 5, Table::ALIGN_RIGHT } });
  table.setContents({ { Table::Cell("Alice"), Table::Cell(30), Table::Cell("New York"), Table::Cell(3.14) },
                      { Table::Cell("Bob"), Table::Cell(25), Table::Cell("Seoul"), Table::Cell(2.71) } });

  std::string output = table.string();

  printf("%s", output.c_str());

  // 핵심 확인: 타이틀 포함 여부
  assert(output.find("Test Table") != std::string::npos);

  // 헤더 존재
  assert(output.find("Name") != std::string::npos);
  assert(output.find("Age") != std::string::npos);
  assert(output.find("City") != std::string::npos);

  // 콘텐츠 존재
  assert(output.find("Alice") != std::string::npos);
  assert(output.find("Bob") != std::string::npos);

  std::cout << "[✔] test_basic_table passed\n";
}

void test_row_divider()
{
  Table table;
  table.setTitle("Divider Test");
  table.setHeaders({ { "Key", 6 },
                     { "Value", 10 } });
  table.setContents({ { Table::Cell("A"), Table::Cell(123) },
                      { Table::Cell("B"), Table::Cell(456) } });
  table.optionRowDivider(true);

  std::string output = table.string();

  // 두 줄 사이에 구분선 확인
  size_t first_row  = output.find("│ A");
  size_t divider    = output.find("├", first_row);
  size_t second_row = output.find("│ B", divider);

  assert(first_row != std::string::npos);
  assert(divider != std::string::npos);
  assert(second_row != std::string::npos);
  assert(divider < second_row);

  std::cout << "[✔] test_row_divider passed\n";
}

void test_header_off()
{
  Table table;
  table.setTitle("No Header");
  table.setHeaders({ { "Label", 10 },
                     { "Data", 10 } });
  table.setContent({ Table::Cell("X"), Table::Cell("100") });
  table.optionShowHeader(false);

  std::string output = table.string();

  // 헤더 텍스트가 출력에 포함되지 않아야 함
  assert(output.find("Label") == std::string::npos);
  assert(output.find("Data") == std::string::npos);

  // 콘텐츠는 포함되어야 함
  assert(output.find("X") != std::string::npos);
  assert(output.find("100") != std::string::npos);

  std::cout << "[✔] test_header_off passed\n";
}

int main()
{
  test_basic_table();
  test_row_divider();
  test_header_off();

  std::cout << "\nAll tests passed!\n";
  return 0;
}
