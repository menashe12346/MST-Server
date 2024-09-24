#ifndef THREADPOOL_H 
#define THREADPOOL_H

#include <iostream>
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>

using namespace std;

/**
 * ThreadPool class that manages a pool of worker threads to execute tasks asynchronously.
 */
class ThreadPool {
public:
    /**
     * Constructor that creates a thread pool with the specified number of threads.
     * @param numThreads - Number of threads to be created in the pool.
     */
    ThreadPool(size_t numThreads);

    /**
     * Destructor that joins all threads and cleans up resources.
     */
    ~ThreadPool();

    /**
     * Method to enqueue a task into the thread pool.
     * @param task - A function representing the task to be executed.
     */
    void enqueue(function<void()> task);

    /**
     * Method to stop all threads in the pool.
     * Ensures that no more tasks are processed once the pool is stopped.
     */
    void stop();

private:
    vector<thread> workers;  // Vector to hold worker threads
    queue<function<void()>> tasks;  // Task queue to hold pending tasks

    mutex queueMutex;  // Mutex to ensure thread-safe access to the task queue
    condition_variable condition;  // Condition variable to notify worker threads of new tasks
    atomic<bool> stopFlag;  // Flag to indicate whether the thread pool should stop

    /**
     * Worker thread function that processes tasks from the task queue.
     */
    void worker();
};

#endif // THREADPOOL_H
