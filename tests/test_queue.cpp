#include <gtest/gtest.h>
#include <spdlog/spdlog.h>
#include "WorkStealingBoundedQueue.h"

TEST(StealingBoundedQueue, Overflow)
{
  JobSystem::WorkStealingBoundedQueue<int> queue(16);
  for (size_t i = 0; i < queue.Capacity(); ++i) { ASSERT_TRUE(queue.Push(i)); }

  ASSERT_FALSE(queue.Push(-1));
}

TEST(StealingBoundedQueue, Undeflow)
{
  JobSystem::WorkStealingBoundedQueue<int> queue(16);
  for (size_t i = 0; i < queue.Capacity(); ++i) { ASSERT_TRUE(queue.Push(i)) << i; }

  int dummy = 0;
  for (size_t i = 0; i < queue.Capacity(); ++i) { ASSERT_TRUE(queue.Pop(dummy)) << i; }

  ASSERT_FALSE(queue.Pop(dummy));
}

TEST(StealingBoundedQueue, Steal)
{
  JobSystem::WorkStealingBoundedQueue<int> queue(16);

  ASSERT_TRUE(queue.Push(1));
  ASSERT_TRUE(queue.Push(2));
  //  ASSERT_TRUE(queue.Steal(-1));

  int dummy = 0;
  ASSERT_TRUE(queue.Pop(dummy));
  ASSERT_EQ(-1, dummy);
  ASSERT_TRUE(queue.Pop(dummy));
  ASSERT_EQ(1, dummy);
  ASSERT_TRUE(queue.Pop(dummy));
  ASSERT_EQ(2, dummy);
}


void BazdKiASzart(int pInt[], size_t capacity)
{
  for (size_t i = 0; i < capacity; ++i) { spdlog::trace(" {}", i, pInt[i]); }
}

TEST(StealingBoundedQueue, MiAFasztCsinal)
{
  int sequence[16];
  JobSystem::WorkStealingBoundedQueue<int> queue(16);

  for (size_t i = 0; i < queue.Capacity(); ++i) {
    ASSERT_TRUE(queue.Push(i)) << i;
    queue.DebugDump(sequence);
    spdlog::trace("Push: {}", i);
    BazdKiASzart(sequence, queue.Capacity());
  }

  for (size_t i = 0; i < queue.Capacity(); ++i) {
    int dummy = 0;
    ASSERT_TRUE(queue.Pop(dummy)) << i;
    queue.DebugDump(sequence);
    spdlog::trace("Pop: {}", i);
    BazdKiASzart(sequence, queue.Capacity());
  }
}
