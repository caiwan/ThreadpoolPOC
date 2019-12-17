#include <random>
#include "ThreadPool.h"

JobSystem::ThreadPool::ThreadPool(const size_t numThreads) : mAllocator(sizeof(Job), numThreads * ThreadPool::maxJobCount, 16), mainThreadId(std::this_thread::get_id())
{
  mNumWorkers = numThreads;
  mWorkers = new Worker *[mNumWorkers + 1];
  for (size_t i = 0; i < mNumWorkers + 1; ++i) { mWorkers[i] = new Worker(); }

  for (size_t i = 0; i < mNumWorkers; ++i) {
    mThreads[i] = std::thread([this]() {
      Worker * worker = FindWorker();
      assert(worker);
      while (worker->mIsTerminated) {
        Job * job = GetJob();
        if (job) { Execute(job); }
      }
    });
  }
}

JobSystem::ThreadPool::~ThreadPool()
{
  for (size_t i = 0; i < mNumWorkers; ++i) { mWorkers[i]->mIsTerminated.store(true); }
  for (size_t i = 0; i < mNumWorkers - 1; ++i) { mThreads[i].join(); }
  for (size_t i = 0; i < mNumWorkers - 1; ++i) { delete mWorkers[i]; }
  delete[] mWorkers;
}

JobSystem::Job * JobSystem::ThreadPool::CreateJob(JobSystem::JobFunction function)
{
  Job * job = AllocateJob();
  job->function = function;
  job->parent = nullptr;
  job->unfinishedJobs = 1;

  return job;
}

JobSystem::Job * JobSystem::ThreadPool::CreateJobAsChild(JobSystem::Job * parent, JobSystem::JobFunction function)
{
  parent->unfinishedJobs++;

  Job * job = AllocateJob();
  job->function = function;
  job->parent = parent;
  job->unfinishedJobs = 1;

  return job;
}

void JobSystem::ThreadPool::Schedule(JobSystem::Job * pJob)
{
  size_t randomIndex = std::rand() % (mNumWorkers + 1);
  mWorkers[randomIndex]->mQueue.Push(pJob);
}


JobSystem::Job * JobSystem::ThreadPool::AllocateJob()
{
  Job * job = reinterpret_cast<Job *>(mAllocator.Allocate());
  assert(job);
  return job;
}

void JobSystem::ThreadPool::Deallocate(JobSystem::Job * pJob) { mAllocator.Deallocate(pJob); }

void JobSystem::ThreadPool::Steal(JobSystem::JobQueue *& stolenQueue)
{
  size_t randomIndex = std::rand() % (mNumWorkers);
  stolenQueue = &mWorkers[randomIndex]->mQueue;
}

void JobSystem::ThreadPool::Wait(JobSystem::Job * pJob)
{
  // wait until the job has completed. in the meantime, work on any other pJob.
  while (!HasJobCompleted(pJob)) {
    Job * nextJob = GetJob();
    if (nextJob) { Execute(nextJob); }
  }
}

JobSystem::Worker * JobSystem::ThreadPool::FindWorker()
{
  const auto threadId = std::this_thread::get_id();
  if (threadId == mainThreadId) { return mWorkers[0]; }

  for (size_t i = 0; i < mNumWorkers; ++i) {
    if (mThreads[i].get_id() == threadId) { return mWorkers[i + 1]; }
  }
  return nullptr;
}

// void JobSystem::ThreadPool::operator()() {}

JobSystem::Job * JobSystem::ThreadPool::GetJob()
{
  Worker * worker = FindWorker();
  assert(worker);
  Job * job = nullptr;
  const bool hasJob = !worker->mQueue.Pop(job);
  if (!hasJob) {
    // this is not a valid job because our own queue is empty, so try stealing from some other queue

    JobQueue * stolenQueue = nullptr;
    Steal(stolenQueue);

    if (stolenQueue == &worker->mQueue) {
      // don't try to steal from ourselves
      Yield();
      return nullptr;
    }

    Job * stolenJob = nullptr;
    const bool hasStolenJob = stolenQueue->Pop(stolenJob);
    if (!hasStolenJob) {
      // we couldn't steal a job from the other queue either, so we just yield our time slice for now
      Yield();
      return nullptr;
    }

    return stolenJob;
  }

  return job;
}

void JobSystem::ThreadPool::Execute(JobSystem::Job * pJob)
{
  // TODO
  //  (pJob->function)(pJob, pJob->data);
  //  Finish(pJob);
}

void JobSystem::ThreadPool::Finish(JobSystem::Job * pJob)
{
  pJob->unfinishedJobs--;
  auto unfinishedJobs = pJob->unfinishedJobs.load(std::memory_order_relaxed);
  if (unfinishedJobs == 0) {
    if (pJob->parent) { Finish(pJob->parent); }
    Deallocate(pJob);
  }
}

void JobSystem::ThreadPool::Yield() noexcept { std::this_thread::yield(); }

bool JobSystem::ThreadPool::HasJobCompleted(const JobSystem::Job * pJob)
{
  const auto unfinishedJobs = pJob->unfinishedJobs.load(std::memory_order_relaxed);
  return unfinishedJobs == 0;
}

// ------------------------------------------------------------------------------------------------------------------

JobSystem::Worker::Worker() : mQueue(ThreadPool::maxJobCount) { mIsTerminated.store(false); }
