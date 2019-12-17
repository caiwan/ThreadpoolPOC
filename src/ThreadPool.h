#pragma once

#include <cstddef>
#include <functional>
#include <mutex>
#include <future>
#include <vector>
#include <queue>
#include "BoundedMpmcQueue.h"
#include "MemoryPoolAllocator.h"

namespace JobSystem
{
  struct Job;
  class Worker;

  typedef void (*JobFunction)(Job *, const void *);

  typedef BoundedMpmcQueue<Job *> JobQueue;

  constexpr size_t cachelineSize = 64;
  typedef char CachelinePadType[cachelineSize];

  class ThreadPool;

  class ThreadPool
  {
    friend class Worker;

  public:
    static const size_t maxJobCount = 4096;

    explicit ThreadPool(size_t numThreads);
    virtual ~ThreadPool();

    template<typename FunctionType, typename... Args> std::future<typename std::result_of<FunctionType(Args...)>::type> Async(FunctionType && job, Args &&... args);

    ThreadPool(const ThreadPool &) = delete;
    ThreadPool & operator=(const ThreadPool &) = delete;

    size_t NumWorkers() const { return mNumWorkers; }

    Job * CreateJob(JobFunction function);
    Job * CreateJobAsChild(Job * parent, JobFunction function);

    void Schedule(Job * pJob);
    void Wait(Job * pJob);

  protected:
    Job * AllocateJob();
    void Deallocate(Job * pJob);
    void Steal(JobQueue *& stolenQueue);

    Worker * FindWorker();

    Job * GetJob();

    void Execute(Job * pJob);
    void Finish(Job * pJob);

    void Yield() noexcept;

    bool HasJobCompleted(const Job * pJob);

    //    void operator()();

  private:
    size_t mNumWorkers;
    Worker ** mWorkers;

    MemoryPoolAllocator mAllocator;

    std::thread::id mainThreadId;

    std::thread * mThreads;
  };

  struct Worker
  {
    Worker();
    JobQueue mQueue;
    std::atomic_bool mIsTerminated;
  };

  struct Job
  {
    JobFunction function;
    Job * parent;
    std::atomic_char32_t unfinishedJobs;
    CachelinePadType padding;
  };

  // ------------------------------------------------------------------------------------------------------------------

  template<typename FunctionType, typename... Args> std::future<typename std::result_of<FunctionType(Args...)>::type> ThreadPool::Async(FunctionType && job, Args &&... args)
  {
    using ReturnType = typename std::result_of<FunctionType(Args...)>::type;
    auto task = std::make_shared<std::packaged_task<ReturnType()>>(std::bind(std::forward<FunctionType>(job), std::forward<Args>(args)...));
    std::future<ReturnType> res = task->get_future();

    // TODO:
    //    if (mIsStop) throw std::runtime_error("Cannot add new tasks to a stopped ThreadPool");
    //    const Job taskWrapper({ [task]() { (*task)(); } });
    //    if (!mQueue.Push(taskWrapper)) throw std::runtime_error("bounded task queue is full you cannot add more.");
    //    return res;
  }


} // namespace JobSystem
