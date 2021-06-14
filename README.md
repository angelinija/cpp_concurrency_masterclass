
# Section 1 -- Thread Management Guide

## Process
* A *process* is an instance of a computer program that is being executed

## Thread
* A *thread* of execution is the smallest sequence of programmed instructions that can be managed independently by a scheduler
* A thread is a component of a process. Every process has at least one thread called the main thread which is the entrypoint for the program

> The typical difference between threads and processes is that threads (of the same process) run in a shared memory space, while processes run in separate memory spaces

## Joinability
* A *properly constructed* thread object represents an active thread of execution in the hardware level. Such a thread object is *joinable*
* For any joinable thread, we must either call the *join* or *detach* function
* After we make such a call, that thread object becomes *non joinable*
* If you forgot to join or detach a joinable thread, then the *`std::terminate`* function will be called when the destructor is called
* If any program has a `std::terminate` call then we refer to such program as an *unsafe program*

## join()
* Joins introduce a *synchronize point* between the launched thread and the thread that it launched from
* It *blocks the execution* of the thread that calls `join()` until the launched thread's execution finishes
```
Thread 1 execution
       |
       |
       |
join() +------> Thread 2 execution
              |
              |
              |
              v
       +------+
       |
       |
       v
```

## detach()
* *Separates* the launched thread from th thread object which it launched from, allowing execution to continue *independently*
* Any allocated resources will be freed once the thread exits
```
Thread 1 execution
         |
         |
         |
detach() +------> Thread 2 execution
         |      |
         |      |
         |      |
         |      |
         |      v
         v
```

## Join in Exception Scenarios
* We can call `detach()` as soon as we launch a thread, as it *does not block* the calling thread
* In some occasions, we cannot call `join()` as soon as we launch a thread, as it *blocks* the calling thread
* RAII - Resource acquisition is initialization
  * Constructor acquires resources, destructor releases resources
  ` thread_guard` class

## get_id()
* Returns a unique thread id for each active thread of execution
* Returns 0 for all non-active threads

## sleep_for()
* Blocks the execution of the current thread for at least the specified `sleep_duration`
* This function may block for longer than `sleep_duration` due to scheduling or resource contention delays

## yield()
* `std::this_thread::yield();`
* Yield will give up the current time slice and reinsert the thread into the scheduling queue. The amount of time that expires until the thread is executed again is usually entirely dependent upon the scheduler

## Hardware Concurrency
* Returns the number of concurrent threads supported by the implementation. The values should be considered only a hint
* `std::thread::hardware_concurrency()`

## Thread Local Storage
* When you declare a variable `thread_local` then each thread is going to have its own, distinct, object
* The storage duration is the entire execution of the thread in which it was created, and the value stored in the object is initialized when the thread is started


# Section 2 -- Thread Safe Access to Shared Data and Locking Mechanisms

## Invariants
* Statements that are always strue for a particular data structure
  * For a list data structure, the size variable contains the number of elements in the list
  * In the doubly linked list data structure, if you follow a next pointer from node A to node B, previous pointer in B should point to node A
* Most common problems of multithreaded applications is broken invariants while updating

## Race Condition
* In concurrency, a race condition is anything where the outcome depends on the relative order of execution of operations on two or more threads
* Many times this ordering doesn't matter
* But if this race condition results in broken data structures, then we refer to such race conditions as problematic race conditions

### Note about std::list
* List is not a thread safe data structure
* In fact, most if not all data structures in the STL are not thread safe at all
* Pushing an element to a list is not an atomic operation
  * Creation of new node
  * Setting that node's next pointer to current head node
  * Changing head pointer to point to new node

## lock_guard
* The class `lock_guard` is a mutex wrapper that provides a convenient RAII-style mechanism for owning a mutex for the duration of a scoped block
* When a `lock_guard` object is created, it attempts to take ownership of the mutex it is given. When control leaves the scope in which the `lock_guard` object was created, the `lock_guard` is destructed and the mutex is released

## Things to Remember When Using Mutexes
* Returning a pointer or reference to the protected data
* Passing code to the protected data structure which you don't have control over

# unique_lock
* `unique_lock` is a general purpose mutex ownership wrapper
* It can be used to manage mutexes like we did with `lock_guard` objects
* Unlike `lock_guard` objects, it does not have to acquire the lock for the associated mutex during construction
* `unique_locks` are neither copy constructible nor copy assignable, but they are move constructible and move assignable

# Section 3 -- Communication Between Threads Using Condition Variables and Futures
## Condition Variable
* The condition variable is a basic mechanism for waiting for an event to be triggered by another thread
* The condition variable is associated with some event, and one or more threads can wait for that event to happen. If some thread has determined that event is satisfied, it can then notify one or more of the threads waiting for that condition variable, and wake them up and allow them to continue processing
* Condition variable wake up can be due to:
  * Notification from another thread
  * Can be spurious wake

### Bus Analogy
* Event - arriving to the destination
* Waiting threads - sleeping passenger
* The thread that determines if the event is satisfied and notifies waiting threads - bus driver

## Asynchronous Operations in C++
* Can be created via `std::async(std::launch_policy, Function&& f, Args&&... args);`
* Launch policy can be:
  * `std::launch::async` - Launch in separate thread
  * `std::launch::deferred` - Run in the current thread when `.get()` is called
  * Can specify both separated by `|`, compiler will decide which way to run the async task
    * Can be used to prevent from spawning more threads than the hardware allows

## packaged_task
* The class template `std::package_task` wraps any callable target so that it can be invoked asynchronously
* Its return value or exception thrown, is stored in a shared state which can be accessed through `std::future` objects 
* `class packaged_task<R(Args...)>;`
* e.g, `std::packaged_task<int(int, int)> task(callable object)`

## std::promise
* Each `std::promise` object is paired with a `std::future` object
* A thread with access to the `std::future` object can wait for the result to be set, while another thread that has access to the corresponding `std::promise` object can call `set_value()` to store the value and make the future ready

# Section 4 -- Lock Based Thread Safe Data Structures and Algorithm Implementation
## Parallel STL
* With C++17, we can specify algorithms with a user preferred way of execution, either parallel or sequential
* `std::sort(std::execution::par, my_data.begin(), my_data.end());`
* Execution policy affects:
  * Algorithm's complexity
  * Behavior when exceptions are thrown
  * Where, how, and when steps of an algorithm are executed
* Three execution policies:
  * Sequential (seq)
  * Parallel (par)
  * Parallel unsequential (par_unseq)

# Section 5 -- C++20 Concurrency Features
## JThread
* Motivation for adding another form of thread
  * Thread has to explicitly call either `join` or `detach`
  * We cannot interrupt standard threads execution after we launch it
* Thread execution interrupt
  * JThread is working with `stop_token` to achieve interruptibility
  * We can introduce condition checks with `stop_token` to specify interrupt points
    * `void do_something(std::stop_token token) { if (token.stop_requested() { return; } ) }`
    * `std::jthread interrupt_thread(do_something);`
    * `interrupt_thread.request_stop();`
* [JThread Motivation & Implementation](https://www.youtube.com/watch?v=elFil2VhlH8)

## Subroutines vs Coroutines
* Subroutines
  * Normal functions (blocking)
  * Two stages:
    * Invoke - initializes the function call
    * Finalize - resource allocation
* Coroutines
  * Four stages:
    * Invoke - initializes the function call
    * Suspend
    * Resume
    * Finalize - resource allocation
  * Consist of:
    * A promise object
      * User defined `promise_type` object
      * Manipulate inside coroutine
      * Return result via this object
    * Handle
      * Non-owning handle which is used to resume or destroy the coroutine from outside 
    * Coroutine state
      * Heap allocated
      * Contains promise object, arguments to coroutine, and local variables

## Lazy Generator
* Function which generates a sequence of numbers based on the demand from the caller function
```c++
#include <experimental/coroutine>
#include <memory>
#include <iostream>

template<typename T>
struct Generator {

    struct promise_type;
    using handle_type = experimental::coroutine_handle<promise_type>;

    Generator(handle_type h) : coro(h) {}                         // (3)
    handle_type coro;

    ~Generator() {
        if (coro) coro.destroy();
    }
    Generator(const Generator&) = delete;
    Generator& operator = (const Generator&) = delete;
    Generator(Generator&& oth) noexcept : coro(oth.coro) {
        oth.coro = nullptr;
    }
    Generator& operator = (Generator&& oth) noexcept {
        coro = oth.coro;
        oth.coro = nullptr;
        return *this;
    }
    T getValue() {
        return coro.promise().current_value;
    }
    bool next() {                                                // (5)
        coro.resume();
        return not coro.done();
    }
    struct promise_type {
        promise_type() = default;                               // (1)

        ~promise_type() = default;

        auto initial_suspend() {                                 // (4)
            return experimental::suspend_always();
        }
        auto  final_suspend() noexcept {
            return experimental::suspend_always();
        }
        auto get_return_object() {                               // (2)
            return Generator{ experimental::handle_type::from_promise(*this) };
        }
        void return_void() {
            experimental::suspend_never();
        }

        auto yield_value(const T value) {                        // (6) 
            current_value = value;
            return experimental::suspend_always{};
        }
        void unhandled_exception() {
            std::exit(1);
        }
        T current_value;
    };

};

Generator<int> getNext(int start = 0, int step = 1) noexcept {
    auto value = start;
    for (int i = 0;; ++i) {
        co_yield value;
        value += step;
    }
}

int main() {

    std::cout << std::endl;

    std::cout << "getNext():";
    auto gen = getNext();
    for (int i = 0; i <= 10; ++i) {
        gen.next();
        std::cout << " " << gen.getValue();  // (7)
    }

    std::cout << "\n\n";

    std::cout << "getNext(100, -10):";
    auto gen2 = getNext(100, -10);
    for (int i = 0; i <= 20; ++i) {
        gen2.next();
        std::cout << " " << gen2.getValue();
    }

    std::cout << std::endl;
}
```

## Barrier
* A barrier is a synchronization mechanism that makes threads wait until the required number of threads have reached a certain point in code. Once all the threads have reached the barrier, they're all unblocked and may proceed

# Section 6 -- C++ Memory Model and Atomic Operations
## Atomic Operation
* An indivisible operation. We cannot observer such an operation half-done from any thread in the system
* If all operations on a particular type are atomic then we refer to such types as atomic types

## `atomic_flag`
* Basic and intended as basic building blocks only
  * Atomic flag should be initalized with `ATOMIC_FLAG_INIT`
  * Only two functions `clear()` and `test_and_set()`
    * Note, all atomic types provide `is_lock_free()` function, `std::atomic_flag` is the only exception

## `atomic<*>`
* All atomic types are neither copy assignable nor copy constructible
* Can assign non-atomic types and can be constructed with using non-atomic types
* Functions:
  * `is_lock_free`
    * Implemented using locks or not
  * `store`
  * `load`
  * `exchange`
    * Replace the stored value with a new one and atomically retrieve the original one
  * Compare Exchange
    * `bool r = X.compare_exchange_weak(T& expected, T desired)`
      * Compares the value of the atomic variable, `X`, with the supplied expected value, `T& expected`, and stores the supplied desired value, `T 
      desired`, if they are equal. If they are not equal, the expected value is updated with the actual value of the atomic variable. This will return 
      true if the store to the atomic variable is performed successfully and false otherwise.
    * `compare_exchange_weak`
      * Does not guarantee that the store operation will be performed successfully, even if expected value matches the actual value
    * `compare_exchange_strong`
      * Guarantees that the store operation will be performed successfully when expected value matches actual value. But requires more overhead to provide this guarantee.

## Atomic Pointers, `atomic<T*>`
* Atomic<T*> is an atomic form of pointer
  * Does not mean the object pointed to is atomic, but the pointer itself is atomic
* Neither copy constructible nor copy assignable
* But can be constructed and assigned using non-atomic values
* Additional functions:
  * `fetch_add`, `+=`
  * `fetch_sub`, `-=`
  * `++`
  * `--`

## User Defined Types
* Type must have trivial copy-assignment operator
  * Must not have virtual functions
  * Every non-static member should have trivial copy-assignment operator
  * Not a descendent of a virtual base class
    * Base class must use a compiler generated copy-assignment operator
* Type must be bitwise equality comparable

## Relationships
* Writer thread
  ```c++
    data_vector.push_back(3);
    data_ready.store(true);
  ```
  * Happen-before relationship, push before setting flag
* Reader thread
  ```c++
    while(!data_ready) {}
    cout << data_vector[0];
  ```
  * Happen-before relationship, wait for flag before reading
* Inter-thread-happen-before relationship between these two threads, `data_ready` flag provides synchronization between reader/writer threads, achieves synchronized-with relationship

## Memory Ordering
* `memory_order_seq_cst`
  * The easiest memory ordering option, also the default if none is supplied
  * Implies that the behavior of the program is consistent with a simple sequential view of the world
  ```c++
  #include <thread>
  #include <atomic>
  #include <cassert>
  
  std::atomic<bool> x = {false};
  std::atomic<bool> y = {false};
  std::atomic<int> z = {0};
  
  void write_x()
  {
      x.store(true, std::memory_order_seq_cst);
  }
  
  void write_y()
  {
      y.store(true, std::memory_order_seq_cst);
  }
  
  void read_x_then_y()
  {
      while (!x.load(std::memory_order_seq_cst))
          ;
      if (y.load(std::memory_order_seq_cst)) {
          ++z;
      }
  }
  
  void read_y_then_x()
  {
      while (!y.load(std::memory_order_seq_cst))
          ;
      if (x.load(std::memory_order_seq_cst)) {
          ++z;
      }
  }
  
  int main()
  {
      std::thread a(write_x);
      std::thread b(write_y);
      std::thread c(read_x_then_y);
      std::thread d(read_y_then_x);
      a.join(); b.join(); c.join(); d.join();
      assert(z.load() != 0);  // will never happen
  }
  ```
* `memory_order_relaxed`
  * Total opposite to `memory_order_seq_cst`
  * View of the world of the threads does not need to be consistent to each other
  * No restriction on instruction re-ordering
  * Outcome depends on whether your processor is strongly ordered or not
    * IBM power series and ARM processors are known for weakly ordering
  * Useful when atomicity is required but order is not important
    * e.g. incrementing counters
  ```c++
  // Thread 1:
  r1 = y.load(std::memory_order_relaxed); // A
  x.store(r1, std::memory_order_relaxed); // B
  // Thread 2:
  r2 = x.load(std::memory_order_relaxed); // C 
  y.store(42, std::memory_order_relaxed); // D

  // r1 == r2 == 42 is valid because of reordering
  ```
* `memory_order_acquire`
* `memory_order_release`
* `memory_order_acq_rel`
* `memory_order_consume`

## Categorization of Atomic Operations
* Load / acquire operations
  * load
  * memory_order_relaxed, memory_order_consume, memory_order_acquire, memory_order_seq_cst
* Store / release operations
  * store
  * memory_order_relaxed, memory_order_release, memory_order_seq_cst
* Read modify write / acquire release operations
  * exchange, compare_exchange_weak, compare_exchange_strong, fetch_sub, etc.
  * memory_order_relaxed, memory_order_consume, memory_order_acquire, memory_order_release, memory_order_rel_acq, memory_order_seq_cst

## Partial Definition of Synchronize-with relationship
* Suitably tagged atomic store operation on a variable `x` synchronized with suitably tagged atomic load operation on `x` that reads the value stored by the previous store
* Atomic store operation tagged with `memory_order_release` will have synchronized with relationship with atomic read operation tagged with `memory_order_acquire`

## Carries-a-dependency-to Relationship
* Applies within a thread
* If the result of an operation A is used as a an operand for an operation B, then A carries-a-dependency-to B
* Thread 1
```c++
  X* x = new x;
  x->i = 42;
  
  p.store(x, std::memory_order_release);
```
* Thread 2
```c++
  p.load(std::memory_order_consume);
```
* Creates a dependency-ordered-before relationship between thread 1 and thread 2

## Release Sequence
* After a *release operation* `A` is performed on an atomic object `M`, the longest continuous subsequence of the modification on `M` that consists of
  * Writes performed by the *same thread* that performed `A`
  * Atomic read-modify-write operations made to `M` by *any thread*
  is known as *release sequence* headed by `A`

# Section 7 -- Lock Free Data Structures and Algorithms

## Blocking vs Non-blocking
* Algorithms and data structures that use mutexes, condition variables, and futures to synchronize the data are called blocking data structures and algorithms
* Data structures adn algorithms that don't use blocking library functions are said to be non-blocking

## Lock Free vs Wait Free
* With lock free data structures, some thread makes progress with every step
* With wait free data structures, every thread can make progress regardless of what other threads are doing

# Section 8 -- Thread Pools
* On most systems, it is impractical to have a separate thread for every task that can potentially be done with other tasks, but you would still like to take advantage of the available concurrency where possible. A thread pool allows you to accomplish this

## Simple Thread Pool
* When you have work to do, you call a function to put it on the queue of pending work. Each worker thread takes work off the queue, runs the specified task, and then goes back to the queue for more work.
