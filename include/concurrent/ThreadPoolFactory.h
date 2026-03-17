/**
 * @file ThreadPoolFactory.h
 * @brief Factory for creating ThreadPool implementations.
 * 
 * Copyright (c) 2026 Stephen Kouretas. All Rights Reserved.
 *
 * @author Stephen Kouretas <stephen.kouretas@gmail.com>
 * @date Created: January 25, 2026
 */
#ifndef SK_COMMON_CONCURRENT_THREAD_POOL_FACTORY_H
#define SK_COMMON_CONCURRENT_THREAD_POOL_FACTORY_H

#include <concurrent/ThreadPool.h>

namespace sk { namespace common { namespace concurrent {

class ThreadPoolFactory {
public:
    /**
     * @brief Creates a fixed-size thread pool implementation.
     */
    static ThreadPoolPtr createFixedThreadPool(int numThreads);
};

}}}

#endif