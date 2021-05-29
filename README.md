
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

