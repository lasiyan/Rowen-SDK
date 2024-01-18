#include <cmath>
#include <cstdio>
#include <thread>

#include "rowen/core/time.hpp"
#include "rowen/utils/resource.hpp"

void raiseCPUusage()
{
  while (1)
  {
    for (int i = 0; i < 100000000; i++)
    {
      sqrt(i);
    }
    rs::time::sleep(1s);
  }
}

int main()
{
  std::thread th1(raiseCPUusage);
  std::thread th2(raiseCPUusage);
  std::thread th3(raiseCPUusage);

  // 강제 종료 필요
  while (1)
  {
    // CPU
    printf("total CPU Usage: %.1f%%\n", rs::utils::resource::totalCPUusage());
    printf("curr  CPU Usage: %.1f%%\n", rs::utils::resource::processCPUusage());

    // Memory
    auto memory = rs::utils::resource::processMemory();
    printf("total Mem Usage: %.1f%%\n", rs::utils::resource::totalMemoryUsage());
    printf("curr  memory: %lu %lu %lu\n", memory.virtual_, memory.physical_, memory.shared_);

    // GPU
    printf("GPU Usage: %.1f%%\n", rs::utils::resource::totalGPUUsage());

    rs::time::sleep(1s);
  }

  th1.join();
  th2.join();
  th3.join();

  return 0;
}
