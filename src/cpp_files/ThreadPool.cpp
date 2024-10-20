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

// Enqueue a new task with graphId to ensure leader-follower execution
void ThreadPool::enqueue(int graphId, function<void()> task) {
    {
        unique_lock<mutex> lock(queueMutex); // Lock the queue to safely push the task
        tasks.push({graphId, move(task)});   // Push the task with its graphId
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

// Worker function following the Leader-Follower pattern
void ThreadPool::worker() {
    while (true) {
        pair<int, function<void()>> task;

        // Wait for a task to be available
        {
            unique_lock<mutex> lock(queueMutex);
            condition.wait(lock, [this] { return stopFlag || !tasks.empty(); });

            if (stopFlag && tasks.empty()) return; // Exit if stopFlag is set and no tasks are left

            task = move(tasks.front()); // Retrieve the task from the queue
            tasks.pop(); // Remove the task from the queue
        }

        int graphId = task.first;

        // Ensure only one thread processes the same graph
        {
            unique_lock<mutex> lock(queueMutex);
            if (graphLocks[graphId]) {
                tasks.push(move(task)); // Return task to the queue if graph is locked
                continue;
            }
            graphLocks[graphId] = true; // Lock the graph for the current thread
        }

        // Execute the task
        task.second();

        // Release the lock on the graph
        {
            unique_lock<mutex> lock(queueMutex);
            graphLocks[graphId] = false; // Unlock the graph for other threads
        }
    }
}
