# Threadpool

**Under development**

## Introduction

- Who has the best CPU utilization? The game industry!
  - You have 16.66 ms to calulate physics, update animation and render a frame
  - On a mediocre 4-8-core hardware
  - Runs at millions of users, with different hardware, different situations
  - Their findings and methods are battle tested
  - Let's learn it from them.
- Pros
  - Efficient way to predict the job load
  - Efficient way to avoid locs, waits, and context switches
- Cons
  - Bounded (fix sized) queues and pools, in cost of being dynamic
  - Not always memory-efficient
  - Certain way to program things (*TODO:specify*) Lots of optimization.
  - Time framing (60 fps) (*TODO: specify*)

### Lockless-ness

- [This article](http://www.1024cores.net/home/lock-free-algorithms/introduction) Our intent is 
- **Wait-freedom** means that each thread moves forward regardless of external factors like contention from other threads, other thread blocking. 
  - Wait-free algorithms usually use such primitives as `atomic_exchange`, `atomic_fetch_add` - TODO: std::atomic
  -  and they do not contain cycles that can be affected by other threads
- **Lock-freedom** means that a system as a whole moves forward regardless of anything. 
  - It's a weaker guarantee than wait-freedom. Lockfree algorithms usually use `atomic_compare_exchange` primitive - TODO: std::atomic
  - Lock-free is when Execution happens in a `Read -> Modify -> Write` cycle
  - Assumes **no compiler** and **memory reordering**.
- **Obstruction-freedom** guarantee means that a thread makes forward progress only if it does not encounter contention from other threads.
  - *See [the original paper](http://www.cs.brown.edu/%7Emph/HerlihyLM03/main.pdf) by Maurice Herlihy*
- **Termination-safety**
  - Waitfree, lockfree and obstruction-free algorithms provide a guarantee of termination-safety. That is, a terminated thread does not prevent system-wide forward progress.
  
### Atomic operations, `std::atomic`
- See [std::atomic](https://en.cppreference.com/w/cpp/atomic/atomic)
  - `load` and `store`
  - `compare_exchange_strong` and `compare_exchabge_weak` - See [compare_exchange](https://en.cppreference.com/w/cpp/atomic/atomic/compare_exchange)
    
- See [std::memory_order](https://en.cppreference.com/w/cpp/atomic/memory_order) - *TODO: sort these* 
  - **`memory_order_relaxed`** - Relaxed operation: there are no synchronization or ordering constraints imposed on other reads or writes, only this operation's atomicity is guaranteed 
  - **`memory_order_consume`** - A load operation with this memory order performs a consume operation on the affected memory location: no reads or writes in the current thread dependent on the value currently loaded can be reordered before this load. Writes to data-dependent variables in other threads that release the same atomic variable are visible in the current thread. On most platforms, this affects compiler optimizations only.
  - **`memory_order_acquire`** - A load operation with this memory order performs the acquire operation on the affected memory location: no reads or writes in the current thread can be reordered before this load. All writes in other threads that release the same atomic variable are visible in the current thread.
  - **`memory_order_release`** - A store operation with this memory order performs the release operation: no reads or writes in the current thread can be reordered after this store. All writes in the current thread are visible in other threads that acquire the same atomic variable (see Release-Acquire ordering below) and writes that carry a dependency into the atomic variable become visible in other threads that consume the same atomic.
  - **`memory_order_acq_rel`** - A read-modify-write operation with this memory order is both an acquire operation and a release operation. No memory reads or writes in the current thread can be reordered before or after this store. All writes in other threads that release the same atomic variable are visible before the modification and the modification is visible in other threads that acquire the same atomic variable.
  - **`memory_order_seq_cst`** - A load operation with this memory order performs an acquire operation, a store performs a release operation, and read-modify-write performs both an acquire operation and a release operation, plus a single total order exists in which all threads observe all modifications in the same order. 
  
### Basics of design a Job system
- [Molecular matters, lock-free work stealing, part 1](https://blog.molecular-matters.com/2015/08/24/job-system-2-0-lock-free-work-stealing-part-1-basics/)
- Requirements for the new job system were the following: 
  - The base implementation needs to be simpler. Jobs can be quite stupid on their own, but it should be possible to build high-level algorithms such as e.g. parallel_for on top of the implementation.
  - The job system needs to implement automatic load balancing.
  - Performance improvements should be made by gradually replacing parts of the system with lock-free alternatives, where applicable.
  - The system needs to support “dynamic parallelism”: it must be possible to alter parent-child relationships and add dependencies to a job while it is still running. This is needed to allow high-level primitives such as parallel_for to dynamically split the given workload into smaller jobs.

## Design
- For `N` cores we have `N-1` worker threads.
- The main thread also can help with execute jobs
- Each thread has it's local queue
- implementing the concept of Work Stealing
- **Worker**
  - Get job - Do we have jobs left or do have to steal? 
  - Do job
  - Finish job (Notify)
- **Job**
  - Priorities (Low, Normal High)
  - Job function pointer `typedef void (*JobFunction)(Job*, const void*);`
  - Pointer to parent, 
  - unfinished job count, which is atomic
  - Should occupy the whole cache line (padding)
  - `CreateJob` - Alloc + set unfinshed count to 1
  - `CrateJobAsChild` - Alloc + set parend + `atomic::Increment` - Use the STD Counterpart
  - `Wait` for job: While not finished, do another job
  - `Finish` a job: Notify parent and delete (**garbage-collect**) the job. Delete parent if there's no other jobs to wait for as well.
- **Allocation** - [Molecular matters, lock-free work stealing, part 2](https://blog.molecular-matters.com/2015/09/08/job-system-2-0-lock-free-work-stealing-part-2-a-specialized-allocator/)
  - Get rid of `new` and `delete`. 
  - Allocation can be fast in cost of memory - but improves performance
  - [Pool allocator](https://blog.molecular-matters.com/2012/09/17/memory-allocation-strategies-a-pool-allocator/)
    - `O(1)` allocation / deletion time. Objects are randomly created / deleted - due their dynamic nature
    - All allocation and de-allocation done in once, for fixed `N` count objects of fixed `M` size.
    - [Freelist](https://en.wikipedia.org/wiki/Free_list) - Connects all the unallocated memories via a linked list
- **Stealing** - [Molecular matters, lock-free work stealing, part 3](https://blog.molecular-matters.com/2015/09/25/job-system-2-0-lock-free-work-stealing-part-3-going-lock-free/)
  - The **queue** implements - [Molecular matters, lock-free work stealing, part 3](https://blog.molecular-matters.com/2015/09/25/job-system-2-0-lock-free-work-stealing-part-3-going-lock-free/)
    - Queue / Job ownership: *Private* is only accessible for the owning thread, and *Public* for everyone else.
    - `Push()` - Adds a job to the private (LIFO) end of the queue.
    - `Pop()` - Removes a job from the private (LIFO) end of the queue.
    - `Steal()` - Steals a job from the public (FIFO) end of the queue.
    - `Push` and `Pop` of the queue is not concurrent
    - `bottom` - the next available element index - Incremented by `Push()`, decremented by `Pop()`
    - `top` - the next element that can be stolen - Incremented by `Steal()`
    - `Size()` is `bottom - top`, this means that also
    - `IsEmpty` is `bottom == top`
  - Lock-free 
    - `Push()` only modifies `bottom` and cannot executed concurrently, but
    - `Steal()` reads `bottom` and writes `top`. 
- Use of fibers within tasks? - [Naughty dog](http://twvideo01.ubm-us.net/o1/vault/gdc2015/presentations/Gyrling_Christian_Parallelizing_The_Naughty.pdf)
- **Paralell for** - https://blog.molecular-matters.com/2015/11/09/job-system-2-0-lock-free-work-stealing-part-4-parallel_for/
- **Dependencies** - https://blog.molecular-matters.com/2016/04/04/job-system-2-0-lock-free-work-stealing-part-5-dependencies/










    // TODO Increase bottom

## Implementation

## Notes
- [Lock-free programming with modern C++ - Timur Doumler [ACCU 2017]](https://www.youtube.com/watch?v=qdrp6k4rcP4) Presentation, Youtube
- [CppCon 2017: Anthony Williams “Concurrency, Parallelism and Coroutines”](https://www.youtube.com/watch?v=JvHZ_OECOFU) Presentation, Youtube
- [MPMC Implementation Example](http://www.1024cores.net/home/lock-free-algorithms/queues/bounded-mpmc-queue)
- [Job request example](http://www.1024cores.net/home/scalable-architecture/task-scheduling-strategies/scheduler-example)
- [Cache line size](https://stackoverflow.com/questions/7281699/aligning-to-cache-line-and-knowing-the-cache-line-size/7284876)
- [Task Flow](https://github.com/cpp-taskflow/cpp-taskflow) A c++ library
