/**
 * @file FixedThreadPool.cpp
 * @brief Implementation of the FixedThreadPool class.
 * 
 * Copyright (c) 2026 Stephen Kouretas. All Rights Reserved.
 *
 * @author Stephen Kouretas <stephen.kouretas@gmail.com>
 * @date Created: January 25, 2026
 */
#include "FixedThreadPool.h"

namespace sk { namespace common { namespace concurrent {

FixedThreadPool::FixedThreadPool(int numThreads) : m_isShutdown(false){
    std::unique_lock<std::mutex> lock(m_workersMutex);
    for (int i = 0; i < numThreads; ++i) {
        m_workers.emplace_back(&FixedThreadPool::workerLoop, this);
    }
}

FixedThreadPool::~FixedThreadPool() {
    shutdown();
}

void FixedThreadPool::enqueue(std::function<void()> task) {
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        if (m_isShutdown) {
            // This is what EXPECT_THROW is looking for
            throw std::runtime_error("ThreadPool is shut down");
        }
        m_taskQueue.emplace(std::move(task));
    }
    m_cv.notify_one();
}

void FixedThreadPool::workerLoop() {
    while (true) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            // wait releases the lock and re-acquires it when notified
            m_cv.wait(lock, [this] { 
                return m_isShutdown || !m_taskQueue.empty(); 
            });
            
            if (m_isShutdown && m_taskQueue.empty()) return;
            
            task = std::move(m_taskQueue.front());
            m_taskQueue.pop();
        }
        
        if (task) {
            task(); 
        }
    }
}

void FixedThreadPool::shutdown() {
    // If it was already true, exchange returns true and we exit.
    // If it was false, it sets it to true and we proceed with joining.
    if (m_isShutdown.exchange(true)) {
        return;
    }
    
    m_cv.notify_all(); // Wake up all threads to exit
    
    std::unique_lock<std::mutex> lock(m_workersMutex);
    for (auto& worker : m_workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }
    m_workers.clear();
}

size_t FixedThreadPool::getPoolSize() const {
    std::unique_lock<std::mutex> lock(m_workersMutex);
    return m_workers.size();
}

}}} // namespace sk::common::concurrent