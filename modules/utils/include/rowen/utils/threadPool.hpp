#pragma once

#include <functional>
#include <future>
#include <queue>

namespace rs {
namespace utils {

class thread_pool
{
 public:
  thread_pool(size_t min_thread, size_t max_thread, bool release_wait_until_all_jobs_done = false);
  ~thread_pool();

  template <typename Callable, typename... Args>
  std::future<typename std::result_of<Callable(Args...)>::type> insertJob(
      Callable&& func, Args&&... args)
  {
    if (threads_stop_)
    {
      throw std::runtime_error("ThreadPool stoped");
    }

    using return_type = typename std::result_of<Callable(Args...)>::type;

    auto job = std::make_shared<std::packaged_task<return_type()>>(
        std::bind(std::forward<Callable>(func), std::forward<Args>(args)...));

    std::future<return_type> future_job = job->get_future();

    {
      std::unique_lock<std::mutex> lock(job_mutex_);

      bool available_threads = active_threads_ < total_threads_;

      if (!available_threads && total_threads_ < max_threads_)
      {
        threads_.emplace_back([this]() { this->createWorkerThread(); });
        ++total_threads_;
      }

      job_queue_.push([job]() { (*job)(); });
    }

    job_convar_.notify_one();

    return future_job;
  }

  // 현재 작업 중인 스레드 수를 반환하는 함수
  int workingCount() { return active_threads_.load(); }

  // 현재 스레드 풀의 총 스레드 수를 반환하는 함수
  int workerCount() { return total_threads_.load(); }

  // 현재 대기 중인 작업의 수(동작 중인 것 제외)를 반환하는 함수
  int waitingCount() { return job_queue_.size(); }

  // 스레드 풀에서 사용 가능한 총 개수(가변 최대값)를 반환하는 함수
  int maxThreads() { return max_threads_; }

 private:
  void createWorkerThread();

 private:
  std::vector<std::thread> threads_      = {};
  bool                     threads_stop_ = false;

  size_t              min_threads_;
  size_t              max_threads_;
  std::atomic<size_t> active_threads_;
  std::atomic<size_t> total_threads_;
  bool                release_wait_until_all_jobs_done_ = false;

  std::queue<std::function<void()>> job_queue_;
  std::condition_variable           job_convar_;
  std::mutex                        job_mutex_;
};

};  // namespace utils
};  // namespace rs
