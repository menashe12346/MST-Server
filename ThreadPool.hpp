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

class ThreadPool {
public:
    ThreadPool(size_t numThreads);
    ~ThreadPool();

    void enqueue(function<void()> task);
    void stop();

private:
    vector<thread> workers;
    queue<function<void()>> tasks;

    mutex queueMutex;
    condition_variable condition;
    atomic<bool> stopFlag;

    void worker();
};

#endif // THREADPOOL_H
