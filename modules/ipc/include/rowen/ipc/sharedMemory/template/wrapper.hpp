#pragma once

#include <semaphore.h>
#include <sys/mman.h>
#include <unistd.h>

#include <cstddef>
#include <ctime>

#include "typedef.hpp"

namespace rs {
namespace ipc {
namespace shared_memory {

static constexpr void SAFE_DELETE_SEMAPHORE(sem_t*& sem)
{
  if (sem != nullptr)
  {
    ::sem_close(sem);
    sem = nullptr;
  }
}

static void UNLINK_SEMAPHORE(const std::string& name)
{
  if (name.empty() == false)
    ::sem_unlink(name.c_str());
}

template <typename MemoryDataType>
static void SAFE_DELETE_SHARED_MEMORY(MemoryDataType*& shm_ptr, size_t& shm_size)
{
  if (shm_ptr != nullptr)
  {
    ::munmap(shm_ptr, shm_size);
    shm_ptr = nullptr;
  }
}

static void UNLINK_SHARED_MEMORY(const std::string& name)
{
  if (name.empty() == false)
    ::shm_unlink(name.c_str());
}

static void SAFE_DELETE_HANDLE(int& shm_fd)
{
  if (shm_fd != INVALID_HANDLE)
  {
    ::close(shm_fd);
    shm_fd = INVALID_HANDLE;
  }
}

/**
 * @brief Wrapper for sem_timedwait
 * @param sem: semaphore object
 * @param timeout_ms: absolute time (milliseconds)
 * @return 1: success, 0: timeout, -1: error (need to check errno)
 */
static int sem_timedwait(sem_t* sem, int timeout_ms)
{
  if (timeout_ms <= 0)
  {
    if (auto res = ::sem_wait(sem); res == -1)
      return -1;
    else
      return 1;
  }

  struct timespec ts;
  if (clock_gettime(CLOCK_REALTIME, &ts) == -1)
    return -1;

  ts.tv_nsec += (timeout_ms % 1000) * 1000000;
  ts.tv_sec += timeout_ms / 1000;
  if (ts.tv_nsec >= 1000000000)
  {
    ts.tv_nsec -= 1000000000;
    ts.tv_sec++;
  }

  auto res = ::sem_timedwait(sem, &ts);

  if (res == -1 && errno == ETIMEDOUT)
    return 0;
  else if (res == -1)
    return -1;
  else
    return 1;
}

};  // namespace shared_memory
};  // namespace ipc
};  // namespace rs
