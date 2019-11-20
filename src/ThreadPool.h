#pragma once

#include <cstddef>
#include <functional>
#include <mutex>
#include <future>
#include <vector>
#include <queue>
#include "BoundedQueue.h"

class ThreadPool
{
public:
  static const size_t maxTasks = 4096;

  explicit ThreadPool(size_t numThreads);
  virtual ~ThreadPool();

  ThreadPool(const ThreadPool &) = delete;
  ThreadPool & operator=(const ThreadPool &) = delete;

  template<typename FunctionType, typename... Args> std::future<typename std::result_of<FunctionType(Args...)>::type> Async(FunctionType && job, Args &&... args);
  void Stop();

  // Todo: Is queue empty or anything in it done

  struct TaskWrapper
  {
    // some extra stuff can be put here
    std::function<void()> task = nullptr;
  };

  bool IsEmpty();

protected:
  std::vector<std::thread> mWorkers;
  BoundedQueue<TaskWrapper> mMpmcQueue;

  std::atomic_bool mIsStop;
  std::atomic_bool mIsEmpty;
};

ThreadPool::ThreadPool(const size_t numThreads) : mMpmcQueue(ThreadPool::maxTasks)
{
  mIsStop = false;
  mIsEmpty = true;
  for (size_t i = 0; i < numThreads; ++i)
    mWorkers.emplace_back([&] {
      for (;;) {
        if (mIsStop) return;
        TaskWrapper task;
        if (mMpmcQueue.Dequeue(task)) {
          task.task();
        } else {
          mIsEmpty = true;
        }
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
  const TaskWrapper taskWrapper({ [task]() { (*task)(); } });
  if (!mMpmcQueue.Enqueue(taskWrapper)) throw std::runtime_error("bounded task queue is full you cannot add more.");
  mIsEmpty = false;
  return res;
}

void ThreadPool::Stop() { mIsStop = true; }
bool ThreadPool::IsEmpty() { return mIsEmpty; }
