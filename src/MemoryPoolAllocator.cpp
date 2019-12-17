//
// Created by psari on 11. 12. 2019.
//

#include <cstdint>
#include <cassert>
#include <cstdlib>

#include "MemoryPoolAllocator.h"

JobSystem::MemoryPoolAllocator::MemoryPoolAllocator(const std::size_t elementSize, const std::size_t numElements, std::size_t alignment)
{
  mHead.store(nullptr);
  const bool result = AllocatePool(elementSize, numElements, alignment);
  assert(result);
}

JobSystem::MemoryPoolAllocator::~MemoryPoolAllocator() { ReleasePool(); }

bool JobSystem::MemoryPoolAllocator::AllocatePool(const std::size_t elementSize, const std::size_t numElements, const std::size_t alignment)
{
  // guarantees that this pool was not allocated before
  assert(mMemory == nullptr);

  mElementSize = elementSize;
  mAlignment = alignment;

  // element size must be at least the same size of void*
  assert(mElementSize >= sizeof(void *));
  // element size must be a multiple of the alignment requirement
  assert(mElementSize % mAlignment == 0);
  // alignment must be a power of two
  assert((mAlignment & (mAlignment - 1)) == 0);

  // --- allocate
  mPoolSize = (mElementSize * numElements) + alignment;
  mMemory = std::aligned_alloc(mAlignment, mElementSize * numElements);
  if (mMemory == nullptr) return false;

  // ---
  // cast the void* to void**, so the memory is 'interpret' as a buffer of void*
  void ** freeMemoryList = static_cast<void **>(mMemory);
  mHead.store(freeMemoryList);
  // get the address of the end of the memory block
  const auto endAddress = reinterpret_cast<uintptr_t>(freeMemoryList) + (elementSize * numElements);

  for (size_t element = 0; element < numElements; ++element) {
    // get the addresses of the current chunck and the next one
    const auto currAddress = reinterpret_cast<uintptr_t>(freeMemoryList) + element * mElementSize;
    const auto nextAddress = currAddress + mElementSize;
    // cast the address of the current chunk to a void**
    void ** currMemory = reinterpret_cast<void **>(currAddress);
    if (nextAddress >= endAddress) { // last chunk
      *currMemory = nullptr;
    } else {
      *currMemory = reinterpret_cast<void *>(nextAddress);
    }
  }
  return true;
}

void * JobSystem::MemoryPoolAllocator::Allocate() noexcept
{
  assert(mMemory);
  void ** pHead = mHead.load(std::memory_order_relaxed);
  if (pHead != nullptr) {
    void * pBlock = reinterpret_cast<void *>(pHead);
    void ** pNext = static_cast<void **>(*pHead);
    mHead.compare_exchange_weak(pHead, pNext);

    return pBlock;
  }

  return nullptr;
}

void JobSystem::MemoryPoolAllocator::Deallocate(void * pBlock) noexcept
{
  if (pBlock == nullptr) { return; }

  assert(mMemory == nullptr);

  void ** pHead = mHead.load(std::memory_order_relaxed);

  if (pHead == nullptr) { // the free list is empty
    void ** pPrev = reinterpret_cast<void **>(pBlock);
    *pPrev = nullptr;
    mHead.compare_exchange_weak(pHead, pPrev);
  } else {
    void ** ppReturnedBlock = pHead;
    void ** pPrev = reinterpret_cast<void **>(pBlock);
    *pHead = reinterpret_cast<void *>(ppReturnedBlock);
    mHead.compare_exchange_weak(pHead, pPrev);
  }
}

void JobSystem::MemoryPoolAllocator::ReleasePool()
{
  std::free(mMemory);
  mMemory = mHead = nullptr;
}
