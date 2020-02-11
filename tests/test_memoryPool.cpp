
#include <gtest/gtest.h>
#include <thread>
#include <atomic>
#include <spdlog/spdlog.h>

#include <ThreadPool/MemoryPoolAllocator.h>


class MemoryPool : public testing::Test
{};

TEST_F(MemoryPool, allocate)
{
  JobSystem::MemoryPoolAllocator allocator(256, 16, 16);
  void * memory = allocator.Allocate();
  ASSERT_TRUE(memory);
  allocator.Deallocate(memory);
}

TEST_F(MemoryPool, overflow)
{
  JobSystem::MemoryPoolAllocator allocator(256, 16, 16);

  for (size_t i = 0; i < 256; ++i) {
    void * memory = allocator.Allocate();
    ASSERT_TRUE(memory);
  }

  void * memory = allocator.Allocate();
  ASSERT_FALSE(memory);
}

TEST_F(MemoryPool, raceConditions)
{

  // given
  unsigned numThreads = std::thread::hardware_concurrency();
  numThreads = numThreads ? numThreads : 2;

  std::vector<std::thread> threads;
  threads.reserve(numThreads);

  JobSystem::MemoryPoolAllocator allocator(256, 16, 16);

  // when
  for (size_t i = 0; i < numThreads; i++) {
    threads.emplace_back([&]() {
      for (int i = 0; i < 100000; ++i) {
        void * memory = allocator.Allocate();
        ASSERT_TRUE(memory);
        allocator.Deallocate(memory);
        // TODO this particulalry does nothing relevant
      }
    });
  }

  for (auto & thread : threads) { thread.join(); }
}
