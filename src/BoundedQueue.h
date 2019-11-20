#pragma once

#include <cstddef>
#include <cassert>
#include <atomic>


/**
 * Bounded Multi-producer/multi-consumer proority queue
 * http://www.1024cores.net/home/lock-free-algorithms/queues
 * http://www.1024cores.net/home/lock-free-algorithms/queues/bounded-mpmc-queue
 * Tl;dr Fix sized Queue without need of mutex lock that solves producer-consumer stuff maintaining priority
 * @tparam T
 */
template<typename T> class BoundedQueue
{
public:
  BoundedQueue(size_t buffer_size);
  ~BoundedQueue();

  BoundedQueue(BoundedQueue const &) = delete;
  void operator=(BoundedQueue const &) = delete;

  bool Enqueue(T const & data);
  bool Dequeue(T & data);

private:
  struct Cell
  {
    std::atomic<size_t> sequence;
    T data;
  };

  static size_t const cachelineSize = 64;
  typedef char CachelinePadType[cachelineSize];

  // Do NOt change this
  // Especially Do NOT use std::unique_ptr<> and STL container
  CachelinePadType pad0_;
  Cell * const mBuffer;
  size_t const mBufferMask;
  CachelinePadType pad1_;
  std::atomic<size_t> mEnqueuePos;
  CachelinePadType pad2_;
  std::atomic<size_t> mDequeuePos;
  CachelinePadType pad3_;
};

// Do NOT use std::unique_ptr<> and STL container
template<typename T> BoundedQueue<T>::BoundedQueue(size_t buffer_size) : mBuffer(new Cell[buffer_size]), mBufferMask(buffer_size - 1)
{
  assert((buffer_size >= 2) && ((buffer_size & (buffer_size - 1)) == 0));
  for (size_t i = 0; i != buffer_size; i += 1) mBuffer[i].sequence.store(i, std::memory_order_relaxed);
  mEnqueuePos.store(0, std::memory_order_relaxed);
  mDequeuePos.store(0, std::memory_order_relaxed);
}
template<typename T> BoundedQueue<T>::~BoundedQueue() { delete[] mBuffer; }

template<typename T> bool BoundedQueue<T>::Enqueue(const T & data)
{
  Cell * cell;
  size_t pos = mEnqueuePos.load(std::memory_order_relaxed);
  for (;;) {
    cell = &mBuffer[pos & mBufferMask];
    size_t seq = cell->sequence.load(std::memory_order_acquire);
    intptr_t dif = (intptr_t)seq - (intptr_t)pos;
    if (dif == 0) {
      if (mEnqueuePos.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed)) break;
    } else if (dif < 0)
      return false;
    else
      pos = mEnqueuePos.load(std::memory_order_relaxed);
  }

  cell->data = data;
  cell->sequence.store(pos + 1, std::memory_order_release);

  return true;
}

template<typename T> bool BoundedQueue<T>::Dequeue(T & data)
{
  Cell * cell;
  size_t pos = mDequeuePos.load(std::memory_order_relaxed);
  for (;;) {
    cell = &mBuffer[pos & mBufferMask];
    size_t seq = cell->sequence.load(std::memory_order_acquire);
    intptr_t dif = (intptr_t)seq - (intptr_t)(pos + 1);
    if (dif == 0) {
      if (mDequeuePos.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed)) break;
    } else if (dif < 0)
      return false;
    else
      pos = mDequeuePos.load(std::memory_order_relaxed);
  }
  data = cell->data;
  cell->sequence.store(pos + mBufferMask + 1, std::memory_order_release);
  return true;
}
