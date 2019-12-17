#pragma once

#include <cstddef>
#include <atomic>

namespace JobSystem
{
  class MemoryPoolAllocator
  {
  public:
    MemoryPoolAllocator(const std::size_t elementSize, const std::size_t numElements, const std::size_t alignment = 16);

    MemoryPoolAllocator(const MemoryPoolAllocator & alloc) = delete;
    MemoryPoolAllocator & operator=(const MemoryPoolAllocator & rhs) = delete;
    MemoryPoolAllocator(MemoryPoolAllocator && alloc) = delete;
    MemoryPoolAllocator & operator=(MemoryPoolAllocator && rhs) = delete;

    ~MemoryPoolAllocator();

    void * Allocate() noexcept;
    void Deallocate(void * pBlock) noexcept;
    size_t ElementSize() const { return mElementSize; }
    size_t Capacity() const { return mPoolSize; }

  private:
    bool AllocatePool(size_t elementSize, size_t numElements, size_t alignment);
    void ReleasePool();

    size_t mPoolSize = 0;
    size_t mElementSize = 0;
    size_t mAlignment = 0;

    void * mMemory = nullptr;
    std::atomic<void **> mHead;
  };

} // namespace JobSystem
