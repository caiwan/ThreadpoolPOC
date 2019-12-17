#include <gtest/gtest.h>
#include <spdlog/spdlog.h>
#include "BoundedMpmcQueue.h"

TEST(StealingBoundedQueue, Overflow)
{
  JobSystem::BoundedMpmcQueue<int> queue(16);
  for (size_t i = 0; i < queue.Capacity(); ++i) { ASSERT_TRUE(queue.Push(i)); }

  ASSERT_FALSE(queue.Push(-1));
}

TEST(StealingBoundedQueue, Undeflow)
{
  JobSystem::BoundedMpmcQueue<int> queue(16);
  for (size_t i = 0; i < queue.Capacity(); ++i) { ASSERT_TRUE(queue.Push(i)) << i; }

  int dummy = 0;
  for (size_t i = 0; i < queue.Capacity(); ++i) { ASSERT_TRUE(queue.Pop(dummy)) << i; }

  ASSERT_FALSE(queue.Pop(dummy));
}

