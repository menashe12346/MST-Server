#include "../hpp_files/ThreadPool.hpp"

// Constructor
ThreadPool::ThreadPool(size_t numThreads) : stopFlag(false) {
    // Create and start the worker threads
    for (size_t i = 0; i < numThreads; ++i) {
        workers.emplace_back([this] { worker(); }); // Each thread runs the worker function
    }
}

// Destructor
ThreadPool::~ThreadPool() {
    stop(); // Ensure the thread pool is properly stopped and cleaned up
}

// Enqueue a new task
void ThreadPool::enqueue(function<void()> task) {
    {
        unique_lock<mutex> lock(queueMutex); // Lock the queue to safely push the task
        tasks.push(move(task)); // Move the task into the queue
    }
    condition.notify_one(); // Notify one worker thread that a new task is available
}

// Stop the thread pool
void ThreadPool::stop() {
    {
        unique_lock<mutex> lock(queueMutex); // Lock the queue to update stopFlag
        stopFlag = true; // Set the stop flag to true, signaling workers to stop
    }
    condition.notify_all(); // Wake up all worker threads to finish processing
    for (thread &worker : workers) {
        worker.join(); // Wait for all worker threads to finish execution
    }
}

// Worker function
void ThreadPool::worker() {
    while (true) {
        function<void()> task;
        {
            unique_lock<mutex> lock(queueMutex); // Lock the queue to wait for tasks
            condition.wait(lock, [this] { return stopFlag || !tasks.empty(); }); // Wait until there is a task or stopFlag is true
            if (stopFlag && tasks.empty()) return; // Exit if stopping and no tasks left
            task = move(tasks.front()); // Get the next task from the queue
            tasks.pop(); // Remove the task from the queue
        }
        task(); // Execute the task
    }
}
