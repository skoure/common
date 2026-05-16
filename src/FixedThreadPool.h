/**
 * @file FixedThreadPool.h
 * @brief Implementation of the FixedThreadPool class.
 * 
 * Copyright (c) 2026 Stephen Kouretas. All Rights Reserved.
 *
 * @author Stephen Kouretas <stephen.kouretas@gmail.com>
 * @date Created: January 25, 2026
 */
#ifndef SK_COMMON_CONCURRENT_FIXED_THREAD_POOL_H
#define SK_COMMON_CONCURRENT_FIXED_THREAD_POOL_H

#include <concurrent/ThreadPool.h>
#include <queue>
#include <list>
#include <mutex>
#include <condition_variable>
#include <thread>

namespace sk { namespace common { namespace concurrent {

/**
 * @class FixedThreadPool
 * @brief A fixed-size implementation of the ThreadPool interface.
 */
class FixedThreadPool : public ThreadPool {
public:
    explicit FixedThreadPool(int numThreads);
    ~FixedThreadPool() override;

    // Implementation of public interface
    void shutdown() override;
    size_t getPoolSize() const override;

protected:
    // This is called by the ThreadPool.h base class 'submit' method
    void enqueue(std::function<void()> work) override;

private:
    void workerLoop();

    std::queue<std::function<void()>> m_taskQueue;
    std::list<std::thread> m_workers;
    
    // Shared synchronization primitives
    mutable std::mutex m_workersMutex;
    std::mutex m_mutex;
    std::condition_variable m_cv;
    bool m_isShutdown{false};
};

}}} // namespace sk::common::concurrent

#endif // SK_COMMON_CONCURRENT_FIXED_THREAD_POOL_H