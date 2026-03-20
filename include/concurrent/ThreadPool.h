/**
 * @file ThreadPool.h
 * @brief Abstract ThreadPool interface for concurrent task execution.
 * 
 * Copyright (c) 2026 Stephen Kouretas. All Rights Reserved.
 *
 * @author Stephen Kouretas <stephen.kouretas@gmail.com>
 * @date Created: January 25, 2026
 */
#ifndef SK_COMMON_CONCURRENT_THREADPOOL_H
#define SK_COMMON_CONCURRENT_THREADPOOL_H

#include <future>
#include <functional>
#include <memory>
#include <type_traits>

namespace sk {
namespace common {
namespace concurrent {

#if __cplusplus >= 201703L
    // C++17 and later - invoke_result_t
    template<typename F>
    using result_of_t = std::invoke_result_t<F>;
#else
    // C++14 - result_of
    template<typename F>
    using result_of_t = typename std::result_of<F()>::type;
#endif

template<typename F, typename P>
static void invoke_and_set(F& t, P& p, std::true_type) {
    t();
    p->set_value();
}

template<typename F, typename P>
static void invoke_and_set(F& t, P& p, std::false_type) {
    p->set_value(t());
}

template<typename F, typename C, typename P>
static void invoke_and_set(F& t, C& cb, P& p, std::true_type) {
    t();
    p->set_value();
    cb();
}

template<typename F, typename C, typename P>
static void invoke_and_set(F&t, C& cb, P& p, std::false_type) {
    auto result = t();
    p->set_value(result);
    cb(result);
}

/**
 * @brief Abstract interface for ThreadPool implementations.
 */
class ThreadPool {
public:
    virtual ~ThreadPool() = default;

    /**
     * @brief Standard submission: Task only.
     * Use lambdas to bind arguments: pool->submit([=]{ return myFunc(a, b); });
     */
    template<typename F>
    auto submit(F&& task) -> std::future<result_of_t<F>> {
        using return_type = result_of_t<F>;

        auto promise = std::make_shared<std::promise<return_type>>();
        std::future<return_type> res = promise->get_future();

        this->enqueue([t = std::forward<F>(task), p = promise]() mutable {
            try {
                invoke_and_set(t, p, std::is_void<return_type>{});
            } catch (...) {
                p->set_exception(std::current_exception());
            }
        });

        return res;
    }

    /**
     * @brief Hybrid submission: Task + Async Callback.
     * The callback is triggered on the worker thread as soon as the task completes.
     */
    template<typename F, typename C>
    auto submit(F&& task, C&& callback) -> std::future<result_of_t<F>> {
        using return_type = result_of_t<F>;

        auto promise = std::make_shared<std::promise<return_type>>();
        std::future<return_type> res = promise->get_future();

        this->enqueue([t = std::forward<F>(task), cb = std::forward<C>(callback), p = promise]() mutable {
            try {
                invoke_and_set(t, cb, p, std::is_void<return_type>{});
            } catch (...) {
                p->set_exception(std::current_exception());
            }
        });

        return res;
    }

    virtual void shutdown() = 0;
    virtual size_t getPoolSize() const = 0;

protected:
    /**
     * @brief The internal sink for all work. 
     * Implementations like FixedThreadPool only need to override this.
     */
    virtual void enqueue(std::function<void()> work) = 0;
};

using ThreadPoolPtr = std::shared_ptr<ThreadPool>;

} // namespace concurrent
} // namespace common
} // namespace sk

#endif // SK_COMMON_CONCURRENT_THREADPOOL_H