#pragma once

#include <string>

namespace rs {
namespace ipc {
namespace shared_memory {

constexpr int    INVALID_HANDLE = 0;  // also consider using -1
constexpr size_t INVALID_SIZE   = 0;

static const std::string SEM_NAME(const std::string& shared_memory_name, const std::string& sem_type)
{
  return shared_memory_name + "_" + sem_type;
}

};  // namespace shared_memory
};  // namespace ipc
};  // namespace rs
