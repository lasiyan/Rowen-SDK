#pragma once

#include <fcntl.h>
#include <semaphore.h>
#include <sys/mman.h>

#include <cerrno>
#include <cstring>
#include <rowen/core/response.hpp>
#include <rowen/ipc/sharedMemory/detail/wrapper.hpp>

namespace rs {
namespace ipc {
namespace shared_memory {

template <typename MemoryDataType>
class sender
{
  static constexpr auto DEFAULT_FLAGS = O_CREAT | O_RDWR;
  static constexpr auto DEFAULT_MODE  = 0666;

 public:
  sender(const std::string& shm_name    = "",
         size_t             shm_size    = INVALID_SIZE,
         int                flags       = DEFAULT_FLAGS,
         mode_t             mode        = DEFAULT_MODE,
         bool               synchronize = true);
  virtual ~sender();

  response_void create(const std::string& shm_name,
                       size_t             shm_size,
                       bool               synchronize = true);

  response_void create(const std::string& shm_name,
                       size_t             shm_size,
                       int                flags,
                       mode_t             mode,
                       bool               synchronize);

  void destroy();

  bool validate() const;

  response_void write(const MemoryDataType* data,
                      int                   timeout_ms = 0,
                      size_t                datasize   = sizeof(MemoryDataType));

 public:
  std::string     shm_name() const { return shm_name_; }
  int             shm_fd() const { return shm_fd_; }
  size_t          shm_size() const { return shm_size_; }
  MemoryDataType* shm_ptr() const { return shm_ptr_; }

  int         state() const { return error_.status; }
  std::string error() const { return error_.message; }
  const char* cerror() const { return error_.c_str(); }
  bool        synchronized() const { return synchronize_; }

 private:
  rs::response_t  error_    = {};
  std::string     shm_name_ = "";
  int             shm_fd_   = INVALID_HANDLE;
  size_t          shm_size_ = INVALID_HANDLE;
  MemoryDataType* shm_ptr_  = nullptr;

  // semaphore
  sem_t* sem_access_     = nullptr;  // [비동기] 처리를 위한 최소한의 세마포어
  sem_t* sem_write_done_ = nullptr;  // [동기] 데이터 쓰기 완료를 알리는 세마포어
  sem_t* sem_read_done_  = nullptr;  // [동기] 데이터 읽기 완료를 알리는 세마포어

  // synchronize
  bool synchronize_ = true;
};

/*
----------------------------------------------------------------------------------
  Implementation
----------------------------------------------------------------------------------
*/

template <typename T>
sender<T>::sender(const std::string& shm_name, size_t shm_size, int flags, mode_t mode, bool synchronize)
{
  // open shared memory (if all parameters are valid)
  create(shm_name, shm_size, flags, mode, synchronize);
}

template <typename T>
sender<T>::~sender()
{
  destroy();
}

template <typename T>
response_void sender<T>::create(const std::string& shm_name, size_t shm_size, bool synchronize)
{
  return create(shm_name, shm_size, DEFAULT_FLAGS, DEFAULT_MODE, synchronize);
}

template <typename T>
response_void sender<T>::create(const std::string& shm_name, size_t shm_size, int flags, mode_t mode, bool synchronize)
{
  try
  {
    // 파라메터 체크
    if (shm_name.empty())
      throw rs::response_t(rssInvalidParameter, "shared memory `name` is empty");
    shm_name_ = shm_name;

    if (shm_size == INVALID_SIZE)
      throw rs::response_t(rssInvalidParameter, "shared memory `size` is invalid : less than 1");
    shm_size_ = shm_size;

    // 공유 메모리 생성
    shm_fd_ = ::shm_open(shm_name.c_str(), flags, mode);
    if (shm_fd_ <= INVALID_HANDLE)
      throw rs::response_t(rssProgressError, "shm_open : " + std::string(::strerror(errno)));

    // 공유 메모리 사이즈 설정
    if (::ftruncate(shm_fd_, shm_size) < 0)
      throw rs::response_t(rssProgressError, "ftruncate : " + std::string(::strerror(errno)));

    // 공유 메모리 맵핑
    shm_ptr_ = static_cast<T*>(::mmap(0, shm_size, PROT_WRITE, MAP_SHARED, shm_fd_, 0));
    if (shm_ptr_ == MAP_FAILED)
      throw rs::response_t(rssProgressError, "mmap : " + std::string(::strerror(errno)));

    // 세마포어 생성
    synchronize_ = synchronize;
    if (synchronize)
    {
      sem_write_done_ = ::sem_open(SEM_NAME(shm_name_, "write").c_str(), O_CREAT, mode, 0);
      if (sem_write_done_ == SEM_FAILED)
        throw rs::response_t(rssProgressError, "sem_open : write : " + std::string(::strerror(errno)));

      sem_read_done_ = ::sem_open(SEM_NAME(shm_name_, "read").c_str(), O_CREAT, mode, 1);
      if (sem_read_done_ == SEM_FAILED)
        throw rs::response_t(rssProgressError, "sem_open : read : " + std::string(::strerror(errno)));
    }
    else
    {
      sem_access_ = ::sem_open(SEM_NAME(shm_name_, "access").c_str(), O_CREAT, mode, 1);
      if (sem_access_ == SEM_FAILED)
        throw rs::response_t(rssProgressError, "sem_open : access : " + std::string(::strerror(errno)));
    }
  }
  catch (const rs::response_t& e)
  {
    destroy();
    error_.status  = e.status;
    error_.message = "open : " + e.message;
    return error_;
  }
  catch (const std::exception& e)
  {
    destroy();
    error_.status  = rssException;
    error_.message = "open : " + std::string(e.what());
    return error_;
  }

  return rs::response_t();
}

template <typename T>
void sender<T>::destroy()
{
  // close semaphore
  SAFE_DELETE_SEMAPHORE(sem_access_);
  SAFE_DELETE_SEMAPHORE(sem_write_done_);
  SAFE_DELETE_SEMAPHORE(sem_read_done_);

  // close shared memory
  SAFE_DELETE_SHARED_MEMORY(shm_ptr_, shm_size_);

  // close shared memory handle
  SAFE_DELETE_HANDLE(shm_fd_);

  // remove resources
  UNLINK_SEMAPHORE(SEM_NAME(shm_name_, "access"));
  UNLINK_SEMAPHORE(SEM_NAME(shm_name_, "write"));
  UNLINK_SEMAPHORE(SEM_NAME(shm_name_, "read"));
  UNLINK_SHARED_MEMORY(shm_name_);
}

template <typename T>
bool sender<T>::validate() const
{
  if (synchronize_)
    return (shm_ptr_ != nullptr && sem_write_done_ != nullptr && sem_read_done_ != nullptr);
  else
    return (shm_ptr_ != nullptr && sem_access_ != nullptr);
}

template <typename T>
response_void sender<T>::write(const T* data, int timeout_ms, size_t datasize)
{
  try
  {
    if (validate() == false)
    {
      if (auto res = create(shm_name_, shm_size_); res == false)
        throw rs::response_t(rssProgressError, res.message);
    }

    // 파라메터 체크
    if (data == nullptr)
      throw rs::response_t(rssInvalidParameter, "data is nullptr");

    // 세마포어 대기
    if (synchronize_)
    {
      if (sem_read_done_ && sem_timedwait(sem_read_done_, timeout_ms) <= 0)
        throw rs::response_t(rssProgressError, "sem_timedwait : read done : " + std::string(::strerror(errno)));
    }
    else
    {
      if (sem_access_ && sem_timedwait(sem_access_, timeout_ms) <= 0)
        throw rs::response_t(rssProgressError, "sem_timedwait : access : " + std::string(::strerror(errno)));
    }

    // 공유 메모리 쓰기
    memcpy(shm_ptr_, data, datasize);

    // 세마포어 포스트
    if (synchronize_)
    {
      if (sem_write_done_ && ::sem_post(sem_write_done_) < 0)
        throw rs::response_t(rssProgressError, "sem_post : write done : " + std::string(::strerror(errno)));
    }
    else
    {
      if (::sem_post(sem_access_) < 0)
        throw rs::response_t(rssProgressError, "sem_post : access : " + std::string(::strerror(errno)));
    }
  }
  catch (const rs::response_t& e)
  {
    error_.status  = (errno == ETIMEDOUT) ? rssProcessTimeout : e.status;
    error_.message = "write : " + e.message;
    return error_;
  }
  catch (const std::exception& e)
  {
    error_.status  = rssException;
    error_.message = "write : " + std::string(e.what());
    return error_;
  }

  return rs::response_t();
}

};  // namespace shared_memory
};  // namespace ipc
};  // namespace rs
