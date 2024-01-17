#pragma once

#include <thread>

#include "rowen/core.hpp"

class Foo
{
  void threadFunc(int);

 public:
  Foo() = default;
  ~Foo()
  {
    thread_stop_ = true;
    if (thread_.joinable())
      thread_.join();
  }

  rs::Logger& getLogger() { return logger; }

  void run(int repeat, const rs::Logger::Property& prop);

 private:
  std::thread thread_;
  bool        thread_stop_ = false;

  rs::Logger logger;
};
