#include "threadpool.h"
#include <cassert>
#include <mutex>

auto ThreadPool::worker() -> void * {

  while (true) {
    std::unique_lock<std::mutex> lock(_mu);
    // wait for the condition: a non-empty queue

    while (_queue.empty())
      _notEmpty.wait(lock);

    // got the job
    auto work = _queue.front();
    _queue.pop_front();
    lock.unlock();

    // do the work..
    work.fn(work.arg);
  }
  return nullptr;
}
auto ThreadPool::init(size_t numThreads) -> void {

  assert(numThreads > 0);
  _threads.resize(numThreads);

  for (size_t i = 0; i < numThreads; ++i)
    _threads.emplace_back(&ThreadPool::worker, this);
}

auto ThreadPool::queue(WorkFn fn, void *arg) -> void {

  std::unique_lock<std::mutex> lock(_mu);
  _queue.push_back(Work{fn, arg});
  _notEmpty.notify_one();
  lock.unlock();
}