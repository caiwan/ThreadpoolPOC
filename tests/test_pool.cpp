#include <gtest/gtest.h>
#include <future>
#include <unordered_set>

#include <spdlog/spdlog.h>
#include "ThreadPool.h"

constexpr int numThreads = 12;
constexpr int numJobs = 20;

TEST(PoolTest, PoolTest)
{
  // given
  JobSystem::ThreadPool threadPool(numThreads);
  std::vector<std::pair<int, std::future<int>>> results;

  // when
  for (int i = 0; i < numJobs; i++) {
    results.push_back(std::make_pair(i,
      threadPool.Async(std::function<int(int)>([&](const int & param) {
        spdlog::info("Start {}", param);
        usleep(100); // Waste some time
        spdlog::info("End {}", param);
        return param + 1;
      }),
        i)));
  }

  while (!threadPool.IsEmpty()) {}
  threadPool.Stop();

  // then
  for (auto & result : results) { ASSERT_EQ(result.first + 1, result.second.get()); }
}

TEST(PoolTest, DISABLED_PoolOverflowTest)
{
  JobSystem::ThreadPool threadPool(numThreads);

  // given
  // we fill up the queue
  for (int i = 0; i < JobSystem::ThreadPool::maxJobs; i++) {
    EXPECT_NO_THROW(threadPool.Async(std::function<void()>([&]() {
      usleep(1000 * 1000); // waste some time
    })));
  }

  // then
  // add one more
  // expect
  // to be thrown up
  EXPECT_THROW(threadPool.Async(std::function<void()>([&]() {})), std::runtime_error);

  threadPool.Stop();
}


TEST(PoolTest, SampleUsecases)
{
  // Demonstrates some use-cases with different parameters and returns
  JobSystem::ThreadPool threadPool(numThreads);

  threadPool.Async(std::function<int(int)>([](const auto & param) { return param + 1; }), 1);
  threadPool.Async(std::function<int(int, int)>([](const auto & a, const auto & b) { return a + b; }), 1, 2);

  threadPool.Async(std::function<void(int)>([](const auto & param) { return param + 1; }), 1);
  threadPool.Async(std::function<void(int, int)>([](const auto & a, const auto & b) { return a + b; }), 1, 2);

  threadPool.Async(std::function<void(void)>([]() {}));

  threadPool.Async(std::function<int(void)>([]() { return 42; }));

  while (!threadPool.IsEmpty()) {}
  threadPool.Stop();

  SUCCEED();
}
