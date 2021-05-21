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