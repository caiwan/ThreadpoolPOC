#pragma once

#include <cstddef>
#include <cassert>
#include <atomic>

// TODO: set cache line size in compile time
#if 0
#  if 0
#    define SET_CACHELINE(CACHE_LINE_SIZE) __declspec(align(CACHE_LINE_SIZE))
#  else
#    define SET_CACHELINE(CACHE_LINE_SIZE) __attribute__((align(CACHE_LINE_SIZE)))
#  endif
#else
#  define SET_CACHELINE(CACHE_LINE_SIZE)
#endif

namespace JobSystem
{
  /**
   * @Class StealingBoundedQueue
   * @Brief Bounded Single-producer/multi-consumer queue for job system
   * @tparam T type of elements to be stored in the queue
   */
  SET_CACHELINE(64)
  template<typename T> class WorkStealingBoundedQueue
  {
  public:
    /**
     * Constructs a fix sized (bounded) queue
     * @param bufferSize initial size. Must be power of 2.
     */
    WorkStealingBoundedQueue(size_t bufferSize);

    ~WorkStealingBoundedQueue();

    WorkStealingBoundedQueue(WorkStealingBoundedQueue const &) = delete;

    void operator=(WorkStealingBoundedQueue const &) = delete;

    /**
     * Pushes an element to the end of the queue
     * @param data the element we insert
     * @return false if the queue is already full
     */
    bool Push(T const & data);

    /**
     * Pops an element from the front of the queue
     * @param data the element we acquire
     * @return false if the queue is already empty
     */
    bool Pop(T & data);

    // TODO docs
    /**
     *
     * @param data
     * @return false if the queue is already full
     */
    bool Steal(T & data);

    /**
     * @return Is the queue empty
     */
    bool IsEmpty() const noexcept;

    /**
     * @return Number of elements in the queue
     */
    size_t Size() const noexcept;

    /**
     * @return Maximum capacity of the queue
     */
    size_t Capacity() const noexcept;

  private:
    /**
     * Represents a cell inside the queue
     */
    struct Cell
    {
      std::atomic<size_t> sequence; // TODO: Do we really need this? How do we make this LIFO / FIFO dequeue?
      T data;
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
  template<typename T> WorkStealingBoundedQueue<T>::WorkStealingBoundedQueue(size_t bufferSize) : mBuffer(new Cell[bufferSize]), mBufferMask(bufferSize - 1)
  {
    assert((bufferSize >= 2) && ((bufferSize & (bufferSize - 1)) == 0));
    for (size_t i = 0; i != bufferSize; i += 1) mBuffer[i].sequence.store(i, std::memory_order_relaxed);
    mBottom.store(0, std::memory_order_relaxed);
    mTop.store(0, std::memory_order_relaxed);
  }
  template<typename T> WorkStealingBoundedQueue<T>::~WorkStealingBoundedQueue() { delete[] mBuffer; }

  template<typename T> bool WorkStealingBoundedQueue<T>::Push(const T & data)
  {
    const size_t bottom = mBottom.load(std::memory_order_relaxed);
    const size_t top = mTop.load(std::memory_order_relaxed);

    // TODO Is it full already?
    Cell * cell = &mBuffer[bottom & mBufferMask];
    // TODO Increase bottom
    // TODO What do we have to do with sequenceing?
    return false; // TODO Fix this

  }

  template<typename T> bool WorkStealingBoundedQueue<T>::Pop(T & data) {
    const size_t bottom = mBottom.load(std::memory_order_relaxed);
    const size_t top = mTop.load(std::memory_order_relaxed);

    // TODO Is it empty already?
    Cell * cell = &mBuffer[bottom & mBufferMask];
    // TODO Decrease bottom
    // TODO What do we have to do with sequenceing?
    return false; // TODO Fix this
  }

  template<typename T> bool WorkStealingBoundedQueue<T>::Steal(T & data) {
    const size_t bottom = mBottom.load(std::memory_order_relaxed);
    const size_t top = mTop.load(std::memory_order_relaxed);

    // TODO Is it empty already?
    Cell * cell = &mBuffer[top & mBufferMask];
    // TODO Increase top
    // TODO What do we have to do with sequenceing?

    return false; // TODO Fix this
  }

  template<typename T> bool WorkStealingBoundedQueue<T>::IsEmpty() const noexcept
  {
    const size_t bottom = mBottom.load(std::memory_order_relaxed);
    const size_t top = mTop.load(std::memory_order_relaxed);
    return bottom <= top;
  }

  template<typename T> size_t WorkStealingBoundedQueue<T>::Size() const noexcept
  {
    const size_t bottom = mBottom.load(std::memory_order_relaxed);
    const size_t top = mTop.load(std::memory_order_relaxed);
    return bottom >= top ? bottom - top : 0;
  }

  template<typename T> size_t WorkStealingBoundedQueue<T>::Capacity() const noexcept { return mBufferMask + 1; }

} // namespace JobSystem