#include <algorithm>
#include <exception>
#include <iostream>
#include <rowen/utils/threadPool.hpp>

namespace rs {
namespace utils {

thread_pool::thread_pool(size_t min_threads, size_t max_threads, bool release_wait_until_all_jobs_done)
    : min_threads_(min_threads),
      max_threads_(max_threads),
      active_threads_(0),
      total_threads_(min_threads),
      release_wait_until_all_jobs_done_(release_wait_until_all_jobs_done)
{
  threads_.reserve(max_threads);

  for (size_t i = 0; i < min_threads; ++i)
  {
    threads_.emplace_back([this]() { this->createWorkerThread(); });
  }
}

thread_pool::~thread_pool()
{
  {
    std::unique_lock<std::mutex> lock(job_mutex_);
    threads_stop_ = true;
    if (release_wait_until_all_jobs_done_ == false)
    {
      while (!job_queue_.empty())
        job_queue_.pop();
    }
    job_convar_.notify_all();
  }

  for (auto& thread : threads_)
  {
    if (thread.joinable())
      thread.join();
  }
}

void thread_pool::createWorkerThread()
{
  auto id = std::this_thread::get_id();

  while (true)
  {
    std::unique_lock<std::mutex> ulock(job_mutex_);

    job_convar_.wait(ulock, [this]() {
      return (!this->job_queue_.empty() || this->threads_stop_);
    });

    if (threads_stop_ && job_queue_.empty())
    {
      total_threads_--;
      break;
    }

    if (job_queue_.empty() == false)
    {
      std::function<void()> job = std::move(job_queue_.front());
      job_queue_.pop();
      ulock.unlock();

      active_threads_++;

      try
      {
        job();
      }
      catch (const std::exception& e)
      {
        std::cerr << "ThreadPool : std::Exception : " << e.what() << "\n";
      }
      catch (...)
      {
        std::cerr << "ThreadPool : Unknown Exception : "
                  << "\n";
      }

      active_threads_--;
    }

    if (threads_stop_ == false)
    {
      if (job_queue_.empty() && total_threads_ > min_threads_)
      {
        std::unique_lock<std::mutex> ulock(job_mutex_);

        auto it = std::find_if(
            threads_.begin(), threads_.end(),
            [id](const std::thread& t) { return t.get_id() == id; });

        if (it != threads_.end())
        {
          it->detach();
          threads_.erase(it);
          total_threads_--;
          break;  // exit thread
        }
      }
    }
  }
}

}  // namespace utils
}  // namespace rs
