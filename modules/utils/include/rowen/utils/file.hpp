#pragma once

#if (__cplusplus >= __cpp_lib_filesystem)
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-W#warnings"
  #warning "This header is deprecated. Use <filesystem> instead."
  #pragma clang diagnostic pop

#else
  #include <sys/types.h>

  #include <cstdint>
  #include <string>
  #include <vector>

namespace rs {
namespace utils {
namespace file {

// verify that the file exists
bool exist(const std::string& title);

// get target file size
int size(const std::string& title);

// get file data
bool data(const std::string& title, uint8_t* buffer, int size = 0);

// create file
void create(const std::string& file, mode_t mode);

// delete file
int remove(const std::string& file);

// write text to file
void write(const std::string& file, const char* fmt, ...);

// create directory
void createDir(const std::string& path, mode_t mode = 777,
               bool delete_if_exist = false);

std::vector<std::string> list(const std::string& path,
                              bool               only_file = true);
};  // namespace file
};  // namespace utils
};  // namespace rs
#endif
