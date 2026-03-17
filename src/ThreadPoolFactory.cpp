/**
 * @file ThreadPoolFactory.cpp
 * @brief Implementation of the ThreadPoolFactory class.
 * 
 * Copyright (c) 2026 Stephen Kouretas. All Rights Reserved.
 *
 * @author Stephen Kouretas <stephen.kouretas@gmail.com>
 * @date Created: January 25, 2026
 */
#include <concurrent/ThreadPoolFactory.h>
#include "FixedThreadPool.h"

namespace sk { namespace common { namespace concurrent {

ThreadPoolPtr ThreadPoolFactory::createFixedThreadPool(int numThreads) {
    // Returns the implementation cast to the interface pointer
    return std::make_shared<FixedThreadPool>(numThreads);
}

}}} // namespace sk::common::concurrent