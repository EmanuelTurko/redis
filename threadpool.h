#pragma once

#include <condition_variable>
#include <cstddef>
#include <deque>
#include <mutex>
#include <thread>
#include <vector>

using WorkFn = void (*)(void *);
class ThreadPool {
public:
  struct Work {
    WorkFn fn;
    void *arg{};
  };

  ThreadPool(){};
  ThreadPool(size_t numThreads) { init(numThreads); }

  auto worker() -> void *;
  auto init(size_t numThreads) -> void;
  auto queue(WorkFn fn, void *arg) -> void;

private:
  std::vector<std::thread> _threads{};
  std::deque<Work> _queue{};
  std::mutex _mu{};
  std::condition_variable _notEmpty{};
};