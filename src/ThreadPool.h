#pragma once

#include <cstddef>
#include <functional>
#include <mutex>
#include <future>
#include <vector>
#include <queue>
#include "WorkStealingBoundedQueue.h"

namespace JobSystem
{
  struct Job
  {
    // TODO + parent
    // TODO + child task count
    std::function<void()> task = nullptr; // TODO use function pointer
  };

  // TODO utulize
  enum class JobPriority
  {
    Low,
    Normal,
    High
  };

  class ThreadPool
  {
  public:
    static const size_t maxJobs = 4096;

    explicit ThreadPool(size_t numThreads);
    virtual ~ThreadPool();

    ThreadPool(const ThreadPool &) = delete;
    ThreadPool & operator=(const ThreadPool &) = delete;

    template<typename FunctionType, typename... Args> std::future<typename std::result_of<FunctionType(Args...)>::type> Async(FunctionType && job, Args &&... args);

    void Stop();
    bool IsEmpty();

  protected:
    class WorkerThread
    {};

    std::vector<std::thread> mWorkers;
    WorkStealingBoundedQueue<Job> mQueue;

    std::atomic_bool mIsStop;
  };

  ThreadPool::ThreadPool(const size_t numThreads) : mQueue(ThreadPool::maxJobs)
  {
    mIsStop = false;
    for (size_t i = 0; i < numThreads; ++i)
      mWorkers.emplace_back([&] {
        for (;;) {
          if (mIsStop) return;
          Job task;
          if (mQueue.Pop(task)) task.task();
        }
      });
  }

  ThreadPool::~ThreadPool()
  {
    Stop();
    for (std::thread & worker : mWorkers) worker.join();
  }

  template<typename FunctionType, typename... Args> std::future<typename std::result_of<FunctionType(Args...)>::type> ThreadPool::Async(FunctionType && job, Args &&... args)
  {
    using ReturnType = typename std::result_of<FunctionType(Args...)>::type;
    auto task = std::make_shared<std::packaged_task<ReturnType()>>(std::bind(std::forward<FunctionType>(job), std::forward<Args>(args)...));
    std::future<ReturnType> res = task->get_future();

    if (mIsStop) throw std::runtime_error("Cannot add new tasks to a stopped ThreadPool");
    const Job taskWrapper({ [task]() { (*task)(); } });
    if (!mQueue.Push(taskWrapper)) throw std::runtime_error("bounded task queue is full you cannot add more.");
    return res;
  }

  inline void ThreadPool::Stop() { mIsStop = true; }
  inline bool ThreadPool::IsEmpty() { return mQueue.IsEmpty(); }

} // namespace JobSystem
