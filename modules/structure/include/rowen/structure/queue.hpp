#pragma once

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <queue>

namespace rs {

template <typename T>
class queue
{
 public:
  queue()                        = default;
  ~queue()                       = default;
  queue(const queue&)            = delete;
  queue& operator=(const queue&) = delete;

  // 버퍼에 데이터를 추가한다
  template <typename U>
  void push(U&& value)
  {
    std::lock_guard locker(mutex_);
    queue_.emplace(std::forward<U>(value));
    cond_var_.notify_one();
  }

  template <typename... Args>
  void emplace(Args&&... args)
  {
    std::lock_guard locker(mutex_);
    queue_.emplace(std::forward<Args>(args)...);
    cond_var_.notify_one();
  }

  // 버퍼에서 데이터를 가져온다 (Blocking)
  T pop()
  {
    std::unique_lock locker(mutex_);
    cond_var_.wait(locker, [this] { return !queue_.empty(); });
    T value = std::move(queue_.front());
    queue_.pop();
    return value;
  }

  // 버퍼에서 데이터를 가져온다 (Blocking)
  // 대기 시간을 지정할 수 있다 (대기 시간 내에 데이터가 없을 경우 false를 반환한다)
  bool pop(T& value, std::chrono::nanoseconds timeout = std::chrono::nanoseconds::zero())
  {
    std::unique_lock locker(mutex_);

    if (timeout.count() == 0)
    {
      cond_var_.wait(locker, [this] { return !queue_.empty(); });
    }
    else
    {
      if (cond_var_.wait_for(locker, timeout, [this] { return !queue_.empty(); }) == false)
        return false;  // 대기 시간 초과
    }

    value = std::move(queue_.front());
    queue_.pop();
    return true;
  }

  // 버퍼에서 데이터를 가져온다 (Non-Blocking)
  // 단, 반환할 데이터가 없을 경우 false를 반환한다
  bool try_pop(T& value)
  {
    std::lock_guard locker(mutex_);
    if (queue_.empty())
      return false;

    value = std::move(queue_.front());
    queue_.pop();
    return true;
  }

  // 버퍼에 쌓여있는 데이터를 지정된 개수만큼 가져온다 (Non-Blocking)
  // max_items : 최대 가져올 데이터 개수 (0일 경우, 버퍼에 있는 모든 데이터를 즉시 가져온다)
  std::vector<T> pop_batch_async(size_t max_items = 0)
  {
    std::lock_guard locker(mutex_);
    size_t          count = (max_items == 0) ? queue_.size() : std::min(max_items, queue_.size());
    return collect_items(count);
  }

  // 버퍼에 쌓여있는 데이터를 지정된 개수만큼 가져온다 (Blocking)
  // max_items : 최대 가져올 데이터 개수 (0일 경우, 버퍼에 있는 모든 데이터를 즉시 가져온다. 아무 것도 없을 경우, 대기한다)
  // timeout : 대기 시간 (0일 경우, Blocking)
  std::vector<T> pop_batch(size_t max_items = 0, std::chrono::nanoseconds timeout = std::chrono::nanoseconds::zero())
  {
    std::unique_lock locker(mutex_);

    auto deadline = std::chrono::steady_clock::now() + timeout;

    if (max_items == 0 && timeout.count() == 0)
      cond_var_.wait(locker, [this] { return !queue_.empty(); });
    else if (max_items == 0)
      cond_var_.wait_until(locker, deadline, [this] { return !queue_.empty(); });
    else
      cond_var_.wait_until(locker, deadline, [this, max_items] { return queue_.size() >= max_items; });

    size_t count = (max_items == 0) ? queue_.size() : std::min(max_items, queue_.size());
    return collect_items(count);
  }

  // 버퍼의 맨 앞 데이터를 참조한다
  // 단, 반환된 참조를 사용하는 동안, 다른 스레드에서 pop()을 호출하면 반환된 참조가 Dangling Reference가 될 수 있다
  // 따라서, 반환된 참조는 즉시 사용해야 한다.
  const T& front() const
  {
    std::lock_guard locker(mutex_);
    return queue_.front();
  }

  T& front()
  {
    std::lock_guard locker(mutex_);
    return queue_.front();
  }

  // 버퍼가 비어있는지 확인한다
  bool empty() const
  {
    std::lock_guard locker(mutex_);
    return queue_.empty();
  }

  // 버퍼의 크기를 반환한다
  size_t size() const
  {
    std::lock_guard locker(mutex_);
    return queue_.size();
  }

  // 버퍼를 비운다
  void clear()
  {
    std::lock_guard locker(mutex_);
    std::queue<T>   empty;
    std::swap(queue_, empty);
  }

 private:
  // 버퍼의 데이터 수집 (pop_batch 기능 구현)
  std::vector<T> collect_items(size_t count)
  {
    std::vector<T> batch;

    if (count == 0)
      return batch;
    else
      batch.reserve(count);

    for (size_t i = 0; i < count; ++i)
    {
      batch.emplace_back(std::move(queue_.front()));
      queue_.pop();
    }
    return batch;
  }

 private:
  std::queue<T>           queue_;
  std::mutex              mutex_;
  std::condition_variable cond_var_;
};

};  // namespace rs
