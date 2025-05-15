#pragma once

#include <stdint.h>
#include <sys/sysinfo.h>
#include <sys/types.h>

namespace rs {
namespace utils {

// usage : cycle time : 1second
class resource
{
 public:
  //////////////////////////////////////////////////////
  // CPU Usage
  static double totalCPUusage();
  static double processCPUusage();

  // Memory Usage
  struct MemoryInfo
  {
    uint64_t virtual_;
    uint64_t physical_;
    uint64_t shared_;
    int      page_size_;
  };
  static MemoryInfo totalMemory();
  static MemoryInfo processMemory();
  static double     totalMemoryUsage();

  static uint64_t   convertToMB(uint64_t value) { return value / 1024 / 1024; }
  static uint64_t   convertToGB(uint64_t value) { return value / 1024 / 1024 / 1024; }
  static MemoryInfo convertToMB(MemoryInfo& memory_info);
  static MemoryInfo convertToGB(MemoryInfo& memory_info);

  // GPU Usage
  static double totalGPUUsage();

  // Temperature
  static bool temperature(int index, double* temperature_value);

 private:
  // initialize Resource class
  static void setup();

  static int parseLine(char* line);

 private:
  static struct sysinfo memory_information_;
  static clock_t        curr_cpu_clock_;
  static clock_t        system_cpu_time_;
  static clock_t        user_cpu_time_;
  static int            process_count_;
  static bool           initialized_;
};

};  // namespace utils
};  // namespace rs
