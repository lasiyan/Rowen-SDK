#include "rowen/utils/resource.hpp"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/times.h>
#include <unistd.h>

namespace rs {
namespace utils {

struct sysinfo resource::memory_information_;
clock_t        resource::curr_cpu_clock_;
clock_t        resource::system_cpu_time_;
clock_t        resource::user_cpu_time_;
int            resource::process_count_ = 0;
bool           resource::initialized_   = false;

//////////////////////////////////////////////////////
//  CPU Usage
//////////////////////////////////////////////////////
double resource::totalCPUusage()
{
  long long int user, nice, sys, idle, iowait, irq, softirq, steal, guest, guest_nice;
  FILE*         fileStat = fopen("/proc/stat", "r");

  fscanf(fileStat, "cpu %lld %lld %lld %lld %lld %lld %lld %lld %lld %lld",
         &user, &nice, &sys, &idle, &iowait, &irq, &softirq, &steal, &guest, &guest_nice);
  fclose(fileStat);

  long long int Idle    = idle + iowait;
  long long int NonIdle = user + nice + sys + irq + softirq + steal;
  long long int Total   = Idle + NonIdle;

  static long long int PrevTotal = 0;
  static long long int PrevIdle  = 0;

  double CPU_Percentage = 0.0;
  if (PrevTotal != 0)
  {
    long long int diffIdle  = Idle - PrevIdle;
    long long int diffTotal = Total - PrevTotal;
    CPU_Percentage          = (double)(diffTotal - diffIdle) / diffTotal * 100.0;
  }

  PrevTotal = Total;
  PrevIdle  = Idle;

  return CPU_Percentage;
}

double resource::processCPUusage()
{
  if (initialized_ == false)
  {
    setup();
    initialized_ = true;
  }

  struct tms timeSample;
  clock_t    now;
  double     percent;

  now = times(&timeSample);
  if (now <= curr_cpu_clock_ || timeSample.tms_stime < system_cpu_time_ ||
      timeSample.tms_utime < user_cpu_time_)
  {
    // Overflow detection. Just skip this value.
    percent = -1.0;
  }
  else
  {
    percent = (timeSample.tms_stime - system_cpu_time_) +
              (timeSample.tms_utime - user_cpu_time_);
    percent /= (now - curr_cpu_clock_);
    percent /= process_count_;
    percent *= 100;
  }
  curr_cpu_clock_  = now;
  system_cpu_time_ = timeSample.tms_stime;
  user_cpu_time_   = timeSample.tms_utime;

  return percent;
}

//////////////////////////////////////////////////////
//  Memory Usage
//////////////////////////////////////////////////////
resource::MemoryInfo resource::totalMemory()
{
  sysinfo(&memory_information_);
  MemoryInfo memory_info;
  memory_info.virtual_ = memory_information_.totalram;
  memory_info.virtual_ += memory_information_.totalswap;
  memory_info.virtual_ *= memory_information_.mem_unit;

  memory_info.physical_ = memory_information_.totalram;
  memory_info.physical_ *= memory_information_.mem_unit;

  memory_info.shared_ = memory_information_.sharedram;
  memory_info.shared_ *= memory_information_.mem_unit;

  return memory_info;
}

resource::MemoryInfo resource::processMemory()
{
  MemoryInfo memory_info;

#ifndef DETAIL_MEMORY_INFO

  FILE* file = fopen("/proc/self/statm", "r");
  if (!file)
  {
    perror("Error opening file");
    return { 0, 0, 0, getpagesize() };
  }

  if (fscanf(file, "%ld %ld %ld", &memory_info.virtual_, &memory_info.physical_, &memory_info.shared_) != 3)
  {
    perror("Error scanning file");
    return { 0, 0, 0 };
  }
  fclose(file);

#else

  FILE* file = fopen("/proc/self/status", "r");
  if (!file)
  {
    perror("Error opening file");
    return { 0, 0, 0, getpagesize() };
  }

  MemoryInfo memory_info;
  char       line[128];
  while (fgets(line, 128, file) != NULL)
  {
    if (strncmp(line, "VmSize:", 7) == 0)
    {  // 7 : "VmSize:"
      memory_info.virtual_ = parseLine(line);
    }
    else if (strncmp(line, "VmRSS:", 6) == 0)
    {  // 6 : "VmRSS:"
      memory_info.physical_ = parseLine(line);
    }
    else if (strncmp(line, "RssFile:", 8) == 0)
    {  // 8 : "RssFile:"
      memory_info.shared_ = parseLine(line);
    }
  }
  fclose(file);
#endif

  memory_info.virtual_ *= getpagesize();
  memory_info.physical_ *= getpagesize();
  memory_info.shared_ *= getpagesize();
  memory_info.page_size_ = getpagesize();

  return memory_info;
}

double resource::totalMemoryUsage()
{
#ifndef SIMPLE_MEMORY_USAGE
  FILE* file = fopen("/proc/meminfo", "r");
  if (!file)
  {
    return 0.0;
  }

  char line[128];

  double total_memory     = 0.0;
  double free_memory      = 0.0;
  double available_memory = 0.0;

  while (fgets(line, 128, file) != NULL)
  {
    char     key[32];
    uint64_t value = 0;
    if (sscanf(line, "%s %lu", key, &value) == 2)
    {
      if (strcmp(key, "MemTotal:") == 0)
      {
        total_memory = value;
      }
      else if (strcmp(key, "MemFree:") == 0)
      {
        free_memory = value;
      }
      else if (strcmp(key, "MemAvailable:") == 0)
      {
        available_memory = value;
      }

      if (total_memory > 0 && free_memory > 0 && available_memory > 0)
      {
        break;
      }
    }
  }
  fclose(file);

  // double used_memory = total_memory - free_memory; // available을 사용한 것이 top과 근사하게 계산
  double used_memory = total_memory - available_memory;
  return (used_memory / total_memory) * 100.0;

#else

  sysinfo(&memory_information_);
  double total_memory = memory_information_.totalram;
  double free_memory  = memory_information_.freeram +
                       memory_information_.bufferram;
  double used_memory = total_memory - free_memory;
  return (used_memory / total_memory) * 100.0;
#endif
}

resource::MemoryInfo resource::convertToMB(MemoryInfo& memory_info)
{
  memory_info.virtual_  = convertToMB(memory_info.virtual_);
  memory_info.physical_ = convertToMB(memory_info.physical_);
  memory_info.shared_   = convertToMB(memory_info.shared_);

  return memory_info;
}

resource::MemoryInfo resource::convertToGB(MemoryInfo& memory_info)
{
  memory_info.virtual_  = convertToGB(memory_info.virtual_);
  memory_info.physical_ = convertToGB(memory_info.physical_);
  memory_info.shared_   = convertToGB(memory_info.shared_);

  return memory_info;
}

//////////////////////////////////////////////////////
//  GPU Usage
//////////////////////////////////////////////////////

double resource::totalGPUUsage()
{
  constexpr int MIN_USAGE = 0;
  constexpr int MAX_USAGE = 999;

  FILE* file = fopen("/sys/devices/gpu.0/load", "r");
  if (!file)
  {
    return 0.0;
  }

  double gpu_usage = 0.0;

  // get one line from file
  char line[5];
  while (fgets(line, sizeof(line), file) != NULL)
  {
    int usage = atoi(line);
    if (usage >= MAX_USAGE)
    {
      gpu_usage = 99.9;
    }
    else if (usage > MIN_USAGE && usage < MAX_USAGE)
    {
      gpu_usage = (double)usage / 10.0;
    }
  }
  fclose(file);

  return gpu_usage;
}

//////////////////////////////////////////////////////
//  Thermal & Other
//////////////////////////////////////////////////////
bool resource::temperature(int index, double* temperature_value)
{
  *temperature_value = 0.0;

  char therm_path[64];
  memset(therm_path, '\0', sizeof(therm_path));
  sprintf(therm_path,
          "/sys/devices/virtual/"
          "thermal/thermal_zone%u/temp",
          index);

  FILE* filePtr = fopen(therm_path, "r");
  if (!filePtr)
  {
    return false;
  }

  // 16 bytes buffer for reading ASCII encoded value from pseudo file
  char         readBuf[16];
  unsigned int readBytes = 0;
  while (readBytes < 16)
  {
    int curRead = fread(readBuf + readBytes, 1, 16 - readBytes, filePtr);
    if (curRead > 0)
      readBytes += curRead;
    else
      break;
  }
  fclose(filePtr);

  if (readBytes > 0)
    *(readBuf + readBytes - 1) = 0;  // Remove last char '\n'

  *(temperature_value) = atof(readBuf);
  *(temperature_value) /= 1000.f;

  return true;
}

//////////////////////////////////////////////////////
//  Private
//////////////////////////////////////////////////////
void resource::setup()
{
  FILE*      file;
  struct tms timeSample;
  char       line[128];

  curr_cpu_clock_  = times(&timeSample);
  system_cpu_time_ = timeSample.tms_stime;
  user_cpu_time_   = timeSample.tms_utime;

  file           = fopen("/proc/cpuinfo", "r");
  process_count_ = 0;
  while (fgets(line, 128, file) != NULL)
  {
    if (strncmp(line, "processor", 9) == 0)
      process_count_++;
  }
  fclose(file);
}

int resource::parseLine(char* line)
{
  int         i   = strlen(line);
  const char* ptr = line;
  while (*ptr < '0' || *ptr > '9') ptr++;
  line[i - 3] = '\0';
  return atoi(ptr);
}

}  // namespace utils
}  // namespace rs
