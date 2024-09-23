#include "ThreadPool.hpp"

// Constructor
ThreadPool::ThreadPool(size_t numThreads) : stopFlag(false) {
    for (size_t i = 0; i < numThreads; ++i) {
        workers.emplace_back([this] { worker(); });
    }
}

// Destructor
ThreadPool::~ThreadPool() {
    stop();
}

// Enqueue a new task
void ThreadPool::enqueue(function<void()> task) {
    {
        unique_lock<mutex> lock(queueMutex);
        tasks.push(move(task));
    }
    condition.notify_one(); // Notify one worker thread
}

// Stop the thread pool
void ThreadPool::stop() {
    {
        unique_lock<mutex> lock(queueMutex);
        stopFlag = true;
    }
    condition.notify_all(); // Wake up all workers
    for (thread &worker : workers) {
        worker.join(); // Wait for all threads to finish
    }
}

// Worker function
void ThreadPool::worker() {
    while (true) {
        function<void()> task;
        {
            unique_lock<mutex> lock(queueMutex);
            condition.wait(lock, [this] { return stopFlag || !tasks.empty(); });
            if (stopFlag && tasks.empty()) return; // Exit if stopping
            task = move(tasks.front());
            tasks.pop();
        }
        task(); // Execute the task
    }
}
