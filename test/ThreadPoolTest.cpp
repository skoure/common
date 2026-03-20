#include <gtest/gtest.h>
#include <concurrent/ThreadPoolFactory.h>
#include <concurrent/ThreadPool.h>
#include <future>
#include <vector>

#include "FixedThreadPool.h"

using namespace sk::common::concurrent;

// 1. Define the list of implementations to test.
// When a new ThreadPool Implementation is creaeted later, just add it to this list.
using ThreadPoolImplementations = ::testing::Types<FixedThreadPool>;

// 2. Define a Template Fixture
template <typename T>
class ThreadPoolContractTest : public ::testing::Test {
protected:
    ThreadPoolContractTest() {
        // We use a fixed size of 4 for the generic contract tests
        m_pool = std::make_shared<T>(4);
    }

    ~ThreadPoolContractTest() override {
        m_pool->shutdown();
    }

    std::shared_ptr<T> m_pool;
};

TYPED_TEST_SUITE(ThreadPoolContractTest, ThreadPoolImplementations);

// 3. Write TYPED_TESTs. These run for EVERY class in the Implementations list.
/**
 * @brief Verifies that a basic task returns the correct value via std::future.
 */
TYPED_TEST(ThreadPoolContractTest, BasicFutureReturn) {
    auto future = this->m_pool->submit([]() {
        return 42;
    });

    EXPECT_EQ(future.get(), 42);
}

TYPED_TEST(ThreadPoolContractTest, BasicArithmeticTask) {
    int a = 10;
    int b = 20;
    auto future = this->m_pool->submit([a, b]() {
        return a + b;
    });

    EXPECT_EQ(future.get(), 30);
}

/**
 * @brief Verifies that tasks with void return type work correctly.
 */
TYPED_TEST(ThreadPoolContractTest, VoidReturnTypeTask) {
    std::atomic<int> taskValue{0};
    const int expectedValue = 67;
    
    auto future = this->m_pool->submit([&taskValue, expectedValue]() {
        taskValue = expectedValue;
    });

    EXPECT_TRUE(future.valid()); // Ensure we actually got a usable future back
    // For void tasks, get() should not throw and just wait for completion
    future.get();
    EXPECT_EQ(taskValue, expectedValue);
}

TYPED_TEST(ThreadPoolContractTest, MultipleParallelTasks) {
    const int taskCount = 10;
    std::vector<std::future<int>> futures;

    for (int i = 0; i < taskCount; ++i) {
        futures.push_back(this->m_pool->submit([i]() {
            return i * i;
        }));
    }

    for (int i = 0; i < taskCount; ++i) {
        EXPECT_EQ(futures[i].get(), i * i);
    }
}

/**
 * @brief Verifies the Hybrid model: Future returns value AND Callback fires.
 */
TYPED_TEST(ThreadPoolContractTest, HybridCallbackTest) {
    std::atomic<bool> callbackFired{false};
    std::atomic<int> resultValue{0};

    auto future = this->m_pool->submit(
        []() { return 100; },
        [&](int val) {
            resultValue = val;
            callbackFired = true;
        }
    );

    // Blocking on future.get()
    EXPECT_EQ(future.get(), 100);

    // Ensure callback fired (should be near-instant since it's on the worker thread)
    while (!callbackFired) { std::this_thread::yield(); }
    
    EXPECT_EQ(resultValue, 100);
}


/**
 * @brief Verifies that exceptions thrown in tasks are captured and rethrown by the future.
 */
TYPED_TEST(ThreadPoolContractTest, ExceptionHandlingTest) {
    auto future = this->m_pool->submit([]() -> int {
        throw std::runtime_error("Task Failure");
        return 0;
    });

    EXPECT_THROW(future.get(), std::runtime_error);
}

TYPED_TEST(ThreadPoolContractTest, GracefulShutdownTest) {
    std::atomic<int> completedTasks{0};
    const int taskCount = 10;

    // 1. Queue up tasks that take some time
    for (int i = 0; i < taskCount; ++i) {
        this->m_pool->submit([&completedTasks]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            completedTasks++;
            return 0;
        });
    }

    // 2. Immediately call shutdown. 
    // This should block until all 10 tasks are finished.
    this->m_pool->shutdown();

    // 3. Verify all tasks finished
    EXPECT_EQ(completedTasks.load(), taskCount);
    EXPECT_EQ(this->m_pool->getPoolSize(), 0);
}

/**
 * @brief Verifies that submitting tasks after shutdown throws an exception.
 */
TYPED_TEST(ThreadPoolContractTest, SubmitAfterShutdownTest) {
    this->m_pool->shutdown();

    EXPECT_THROW({
        this->m_pool->submit([]() { return 1; });
    }, std::runtime_error);
}

// 4. Implementation-Specific Tests (Non-Typed)
// These stay as standard tests because they only apply to FixedThreadPool.
TEST(FixedThreadPoolTest, VerifiesExplicitThreadCount) {
    const size_t threads = 7;
    auto pool = ThreadPoolFactory::createFixedThreadPool(threads);
    EXPECT_EQ(pool->getPoolSize(), threads);
    pool->shutdown();
}