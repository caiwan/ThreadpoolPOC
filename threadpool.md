# Threadpool

**Under development**

## Introduction and fundamentals

- Who has the best CPU utilization? The game industry!
  - You have 16.66 ms to calculate physics, update animation and render a frame
  - On a mediocre 4-8-core hardware
  - Runs at millions of users, with different hardware, different situations
  - Their findings and methods are battle tested
  - *Let's learn it from them.*
- **Pros**
  - Efficient way to predict the job load
  - Efficient way to avoid locks, waits, and context switches
- **Cons**
  - Bounded (fix sized) queues and pools, in cost of being dynamic
  - Not always memory-efficient
  - Certain way to program things. (lockless algorythms, splitting large tasks)
  - Time framing (60 fps, 16.66 ms) - You have one frame to collect tasks in the next frame. 

### Lockless-ness

- [This article](http://www.1024cores.net/home/lock-free-algorithms/introduction) **Our intent is** 
- **Wait-freedom** means that each thread moves forward regardless of external factors like contention from other threads, other thread blocking. 
  - Wait-free algorithms usually use such primitives as `atomic_exchange`, `atomic_fetch_add` - TODO: std::atomic
  -  and they do n`ot contain cycles that can be affected by other threads
- **Lock-freedom** means that a system as a whole moves forward regardless of anything. 
  - It's a weaker guarantee than wait-freedom. Lockfree algorithms usually use `atomic_compare_exchange` primitive - TODO: std::atomic
  - Lock-free is when Execution happens in a `Read -> Modify -> Write` cycle
  - Assumes **no compiler** and **memory reordering**.
- **Obstruction-freedom** guarantee means that a thread makes forward progress only if it does not encounter contention from other threads.
  - *See [the original paper](http://www.cs.brown.edu/%7Emph/HerlihyLM03/main.pdf) by Maurice Herlihy*
- **Termination-safety**
  - Wait-free, lock-free and obstruction-free algorithms provide a guarantee of termination-safety. That is, a terminated thread does not prevent system-wide forward progress.
  
### Atomic operations, `std::atomic<T>`
- See [std::atomic reference](https://en.cppreference.com/w/cpp/atomic/atomic) and see [This talk](https://www.youtube.com/watch?v=ZQFzMfHIxng) from Fedor Pikos 
- Memory operations are done in read-modify-write sequence, however there's race condition between stages.
- Atomic operations - done in one single stage, CPU supported,
    - Provides operator overloads only for atomic operations
        - like `++x`, `x++`, `x+=1`, **EXCEPT**
        - no atomic multiply (`x*=2`) 
        - and `x = x +1` is an atomic read followed by write, which is not atommic
        - `bool` has no special operators, as well as `double` - valid and will compile
    - Member functions for better readability 
        - `load()` - Assignment
        - `store()` - Assignment
        - `exchange()` - read-modify-write done atimically
        - `compare_exchange_*(expected, newValue)` - if x==expected then x = new and return true, else returns false. - See [compare_exchange](https://en.cppreference.com/w/cpp/atomic/atomic/compare_exchange)
            - **Key** to *lock-free* algorithms; 
            - `while(!compare_exchange_strong(x, x+1)) {}` - *Note* It's Lock-free but not wait-free!
            - Also allows atomically commit otherwise non-atomic operations, like operations with `double`. 
        - `fetch_*()` - `x.fetch_add(y)` same as `x+=y`, and returns the original value of x before it's change. Also `_sub`, `_and`, `_or`, `_xor` etc.
        - 
    - Performance 
        - Not always guaranteed by different implementations of the platform
        - `is_lock_free()` in runtime  and `is_always_lock_free()` in compile-time
        - Atomics are lock and wait-free, but doesn't necessarily means that they don't wait on ea.
        - Cache-line sharing that prevents atomicity and forces wait
    - Compare and swap operation
        - `compare_exchange_strong(expected, newValue)` - if x==expected then x = new and return true, else returns false. Always grantees this behaviour.
        - `compare_exchange_weak(expected, newValue)` - same, but will *falsely* fail even if comparison is true, depends on hardware implementation, mostly due to timeout.
        - See atomic queue and atomic list [starting from here](https://youtu.be/ZQFzMfHIxng?t=2255)

#### Memory barriers

- Essential part of atomics
- See [std::memory_order](https://en.cppreference.com/w/cpp/atomic/memory_order) - *TODO: sort these* 
- [from here](https://youtu.be/ZQFzMfHIxng?t=2546)
1. **`std::memory_order_relaxed`**
    - there are no synchronization or ordering changes
    - only this operation's atomicity is guaranteed
2. **`std::memory_order_acquire`**
    - grantees that memory operations are scheduled after the barrier
    - both read and write operations
    - all memory operations (not just )
3. **`std::memory_order_release`**  
4. **`std::memory_order_acq_rel`** and **`std::memory_order_seq_cst`** 
  
#### On programmers intent    

- **TBD** 
  
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
  - Yield
- **Job**
  - Priorities comes from the order of insertion
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
    - [Another article](https://medium.com/@mateusgondimlima/designing-and-implementing-a-pool-allocator-data-structure-for-memory-management-in-games-c78ed0902b69) 
- **Stealing** - [Molecular matters, lock-free work stealing, part 3](https://blog.molecular-matters.com/2015/09/25/job-system-2-0-lock-free-work-stealing-part-3-going-lock-free/)
  - The **queue** implements - [Molecular matters, lock-free work stealing, part 3](https://blog.molecular-matters.com/2015/09/25/job-system-2-0-lock-free-work-stealing-part-3-going-lock-free/)
    - StealQueue / Job ownership: *Private* is only accessible for the owning thread, and *Public* for everyone else.
    - `Push()` - Adds a job to the private (LIFO) end of the queue.
    - `Pop()` - Removes a job from the private (LIFO) end of the queue.
    - `Steal()` - Steals a job from the public (FIFO) end of the queue.
    - `Push` and `Pop` of the queue is not concurrent
    - `bottom` - the next available element index - Incremented by `Push()`, decremented by `Pop()`
    - `top` - the next element that can be stolen - Incremented by `Steal()`
    - `Size()` is `bottom - top`, this means that also
    - `IsEmpty` is `bottom == top`
  - [Bounded MPMC queue](http://www.1024cores.net/home/lock-free-algorithms/queues/bounded-mpmc-queue) - **TBD** 
    - `Push()` ~~only modifies `bottom` and cannot executed concurrently, but~~
    - `Pop()`
    - ~~the `Steal()` reads `bottom` and writes `top`.~~
    - *Better to use queu instead of a dequeue;* However It might be better at spawining child-tasks. Similar ideas at
        - [1](https://codereview.stackexchange.com/questions/158711/naive-lock-free-work-stealing-queue) Stackoverfloow 
        - [2](https://www.reddit.com/r/rust/comments/6loior/mpmc_priority_queue/) Reddit /r/rust
        - [3](https://groups.google.com/forum/#!topic/lock-free/glph_p4-dRM) Scalable Synchronization Algorithms *Google Group*
        - *However none of these are completely suitable* 
  - Wait-free and `Yield()`
    - Yields CPU - Makes it sleep for a short amount of time 
    - Heavily platform dependent - [std provides one](https://en.cppreference.com/w/cpp/thread/yield)
  - Thread local - [thread_local](https://en.cppreference.com/w/cpp/keyword/thread_local) keyword, see [storage duration](https://en.cppreference.com/w/cpp/language/storage_duration)
- **TBD**
    - Use of fibers within tasks? - [Naughty dog](http://twvideo01.ubm-us.net/o1/vault/gdc2015/presentations/Gyrling_Christian_Parallelizing_The_Naughty.pdf)
- **Paralell for** - https://blog.molecular-matters.com/2015/11/09/job-system-2-0-lock-free-work-stealing-part-4-parallel_for/
- **Dependencies** - https://blog.molecular-matters.com/2016/04/04/job-system-2-0-lock-free-work-stealing-part-5-dependencies/

## Implementation
- [This simple example](https://github.com/progschj/ThreadPool)
    - Bit more modern approach 
    - Takes lambdas 
    - Wraps them into [`std::packaged_task`](https://en.cppreference.com/w/cpp/thread/packaged_task)
    - Constructs a [`future`](https://en.cppreference.com/w/cpp/thread/future)
    - **Problem**: You need to be able to execute another task if you need to wait for a result. 

## Notes
- [Lock-free programming with modern C++ - Timur Doumler [ACCU 2017]](https://www.youtube.com/watch?v=qdrp6k4rcP4) Presentation, Youtube
- [CppCon 2017: Anthony Williams “Concurrency, Parallelism and Coroutines”](https://www.youtube.com/watch?v=JvHZ_OECOFU) Presentation, Youtube
- [MPMC Implementation Example](http://www.1024cores.net/home/lock-free-algorithms/queues/bounded-mpmc-queue)
- [Job request example](http://www.1024cores.net/home/scalable-architecture/task-scheduling-strategies/scheduler-example)
- [Cache line size](https://stackoverflow.com/questions/7281699/aligning-to-cache-line-and-knowing-the-cache-line-size/7284876)
- [Task Flow](https://github.com/cpp-taskflow/cpp-taskflow) A c++ library
