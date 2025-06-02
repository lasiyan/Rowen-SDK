#include <iostream>
#include <rowen/utils/table.hpp>

using namespace rs;

void runSortingTest()
{
  Table table;

  // 타이틀
  table.setTitle("Sorting Test Table");

  // 1. 헤더 정의
  table.setHeaders({ { "ID", 4, Table::ALIGN_RIGHT },
                     { "Score", 6, Table::ALIGN_RIGHT },
                     { "Active", 6 },
                     { "JoinDate", 12 },
                     { "Timestamp", 26 },
                     { "IP", 16 },
                     { "Name", 8 },
                     { "Remarks", 10 },
                     { "Hex", 8 },
                     { "FileSize", 8, Table::ALIGN_RIGHT },
                     { "SELECT", 0, Table::ALIGN_CENTER } });

  // 2. 데이터 입력 (테스트 데이터셋 복사)
  table.setContents({ { "1", "81.88", "true", "2024-04-18", "2024-04-25 06:12:06.584", "192.168.0.227", "Diana", "Pending", "0x1644", "1996", "[   ]" },
                      { "2", "94.89", "false", "2024-04-29", "2024-04-25 13:49:20.601", "192.168.0.30", "Eve", "On Hold", "0xc6c0", "1416", "[   ]" },
                      { "3", "80.56", "true", "2024-04-14", "2024-04-25 18:14:24.693", "192.168.0.233", "Bob", "Pass", "0x490a", "2673", "[   ]" },
                      { "4", "90.55", "true", "2024-04-10", "2024-04-25 21:39:41.098", "192.168.0.6", "Bob", "On Hold", "0x2290", "2583", "[   ]" },
                      { "5", "68.38", "false", "2024-04-26", "2024-04-25 14:13:15.369", "192.168.0.62", "Bob", "Fail", "0xafd5", "4312", "[ * ]" },
                      { "6", "72.12", "true", "2024-04-21", "2024-04-25 09:45:15.229", "192.168.0.101", "Alice", "Pass", "0xbeef", "1100", "[   ]" },
                      { "7", "85.00", "false", "2024-04-19", "2024-04-25 16:27:32.888", "192.168.0.120", "Charlie", "Pending", "0x1234", "2987", "[   ]" },
                      { "8", "77.77", "true", "2024-04-23", "2024-04-25 17:00:00.000", "192.168.0.88", "Diana", "Pass", "0xabcd", "2050", "[   ]" },
                      { "9", "66.66", "false", "2024-04-15", "2024-04-25 20:00:00.999", "192.168.0.33", "Eve", "Fail", "0x1111", "3800", "[   ]" },
                      { "10", "99.99", "true", "2024-04-28", "2024-04-25 23:59:59.999", "192.168.0.55", "Alice", "On Hold", "0xffee", "1234", "[   ]" } });

  // 3. 테스트 수행
  auto testSort = [&](const std::string& column, bool ascending = true) {
    std::cout << "\n--- Sorted by column: " << column << (ascending ? " (ASC)" : " (DESC)") << " ---\n";
    table.sortedByColumn(column, ascending);
    std::cout << table << std::endl;
  };

  // 4. 정렬 테스트 실행 (10가지 형태)
  testSort("Score");
  testSort("Score", false);
  testSort("Active");
  testSort("JoinDate");
  testSort("Timestamp", false);
  testSort("IP");
  testSort("Name", false);
  testSort("Remarks");
  testSort("Hex");
  testSort("FileSize", false);
  testSort("SELECT", true);
}

#if 0
int main()
{
  runSortingTest();
  return 0;
}
#endif
