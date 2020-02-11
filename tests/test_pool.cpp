#include <gtest/gtest.h>
#include <future>
#include <unordered_set>

#include <spdlog/spdlog.h>

#include <ThreadPool/ThreadPool.h>

using JobSystem::Internal::ThreadPool;
using JobSystem::Internal::Job;

struct TestJobData
{
  size_t counter;
  size_t result;
};


void TestJobFunction(Job * job, void * rawData)
{
  TestJobData * data = reinterpret_cast<TestJobData *>(rawData);
  spdlog::info("Hello from thread {}", data->counter);
  data->result = data->counter + 1;
}

TEST(PoolTest, PoolTest)
{
  constexpr size_t maxJobCount = 256;

  // Given
  const size_t numThreads = std::thread::hardware_concurrency() ? std::thread::hardware_concurrency() : 2;
  ThreadPool threadPool(numThreads);

  JobSystem::MemoryPoolAllocator allocator(maxJobCount, sizeof(TestJobData));

  // When
  Job * jobs[maxJobCount] = {};
  size_t counter = 0;
  for (Job *& job : jobs) {
    auto * data = reinterpret_cast<TestJobData *>(allocator.Allocate());
    ASSERT_TRUE(data);

    data->counter = counter++;
    data->result = 0;

    job = threadPool.CreateJob(&TestJobFunction, data);
    threadPool.Schedule(job);

    ASSERT_TRUE(job);
  }

  // Then
  for (Job *& job : jobs) {
    threadPool.Wait(job);
    auto * data = reinterpret_cast<TestJobData *>(job->data);
    ASSERT_TRUE(data->result = data->counter + 1);
    allocator.Deallocate(job->data);
  }
}
