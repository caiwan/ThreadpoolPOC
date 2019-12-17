#pragma once

#include <cstddef>
#include <cassert>
#include <atomic>

// TODO: set cache line size in compile time
//#if 0
//#  if 0
//#    define SET_CACHELINE(CACHE_LINE_SIZE) __declspec(align(CACHE_LINE_SIZE))
//#  else
//#    define SET_CACHELINE(CACHE_LINE_SIZE) __attribute__((align(CACHE_LINE_SIZE)))
//#  endif
//#else
//#  define SET_CACHELINE(CACHE_LINE_SIZE)
//#endif

namespace JobSystem
{
//  SET_CACHELINE(64)
  template<typename T> class BoundedMpmcQueue
  {
  public:
    BoundedMpmcQueue(size_t bufferSize);
    ~BoundedMpmcQueue();
    BoundedMpmcQueue(BoundedMpmcQueue const &) = delete;

    void operator=(BoundedMpmcQueue const &) = delete;

    bool Push(T const & data);
    bool Pop(T & data);

    bool IsEmpty() const noexcept;

    size_t Size() const noexcept;

    size_t Capacity() const noexcept;

  private:

    struct Cell
    {
      std::atomic<size_t> sequence; // TODO: Do we really need this? How do we make this LIFO / FIFO dequeue?
      T data = {};
    };

    // Memory alignment
    // Do NOT change this
    static size_t const cachelineSize = 64;
    typedef char CachelinePadType[cachelineSize];

    // Do NOT change this
    // Especially Do NOT use std::unique_ptr<> and STL container
    CachelinePadType pad0_{};
    Cell * const mBuffer;
    size_t const mBufferMask;
    CachelinePadType pad1_{};
    std::atomic<size_t> mBottom{};
    CachelinePadType pad2_{};
    std::atomic<size_t> mTop{};
    CachelinePadType pad3_{};
  };

  // Do NOT use std::unique_ptr<> and STL container
  template<typename T> BoundedMpmcQueue<T>::BoundedMpmcQueue(size_t bufferSize) : mBuffer(new Cell[bufferSize]), mBufferMask(bufferSize - 1)
  {
    assert((bufferSize >= 2) && ((bufferSize & (bufferSize - 1)) == 0));
    for (size_t i = 0; i != bufferSize; i += 1) { mBuffer[i].sequence.store(i, std::memory_order_relaxed); }
    mBottom.store(0, std::memory_order_relaxed);
    mTop.store(0, std::memory_order_relaxed);
  }
  template<typename T> BoundedMpmcQueue<T>::~BoundedMpmcQueue() { delete[] mBuffer; }

  template<typename T> bool BoundedMpmcQueue<T>::Push(const T & data)
  {
    Cell * cell = nullptr;
    size_t pos = mBottom.load(std::memory_order_relaxed);
    for (;;) {
      cell = &mBuffer[pos & mBufferMask];
      size_t seq = cell->sequence.load(std::memory_order_acquire);
      intptr_t dif = (intptr_t)seq - (intptr_t)pos;
      if (dif == 0) {
        if (mBottom.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed)) break;
      } else if (dif < 0)
        return false;
      else
        pos = mBottom.load(std::memory_order_relaxed);
    }
    cell->data = data;
    cell->sequence.store(pos + 1, std::memory_order_release); // TODO: What does this particlular line do?
    return true;
  }

  template<typename T> bool BoundedMpmcQueue<T>::Pop(T & data)
  {
    Cell * cell;
    size_t pos = mTop.load(std::memory_order_relaxed);
    for (;;) {
      cell = &mBuffer[pos & mBufferMask];
      size_t seq = cell->sequence.load(std::memory_order_acquire);
      intptr_t dif = (intptr_t)seq - (intptr_t)(pos + 1);
      if (dif == 0) {
        if (mTop.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed)) break;
      } else if (dif < 0)
        return false;
      else
        pos = mTop.load(std::memory_order_relaxed);
    }
    data = cell->data;
    cell->sequence.store(pos + mBufferMask + 1, std::memory_order_release); // TODO: What does this particlular line do?
    return true;
  }

  template<typename T> bool BoundedMpmcQueue<T>::IsEmpty() const noexcept
  {
    const size_t bottom = mBottom.load(std::memory_order_relaxed);
    const size_t top = mTop.load(std::memory_order_relaxed);
    return bottom <= top;
  }

  template<typename T> size_t BoundedMpmcQueue<T>::Size() const noexcept
  {
    const size_t bottom = mBottom.load(std::memory_order_relaxed);
    const size_t top = mTop.load(std::memory_order_relaxed);
    return bottom >= top ? bottom - top : 0;
  }

  template<typename T> size_t BoundedMpmcQueue<T>::Capacity() const noexcept { return mBufferMask + 1; }

} // namespace JobSystem
