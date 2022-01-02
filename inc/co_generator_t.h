#pragma once

#include <coroutine>
#include <utility>

// from https://www.scs.stanford.edu/~dm/blog/c++-coroutines.html

namespace EightBit {

template<typename T>
class co_generator_t final {
public:
    class promise_type;

private:
    using handle_t = std::coroutine_handle<promise_type>;

    class promise_type final {
    private:
        T value_;
        std::exception_ptr exception_;

    public:
        template<std::convertible_to<T> From> // C++20 concept
        [[nodiscard]] constexpr std::suspend_always yield_value(From&& from) noexcept {
            value_ = std::forward<From>(from);
            return {};
        }

        [[nodiscard]] constexpr std::suspend_always initial_suspend() noexcept { return {}; }
        [[nodiscard]] constexpr std::suspend_always final_suspend() noexcept { return {}; }

        [[nodiscard]] constexpr co_generator_t get_return_object() noexcept {
            return co_generator_t(handle_t::from_promise(*this));
        }

        constexpr void return_void() {}

        constexpr void unhandled_exception() noexcept { exception_ = std::current_exception(); }

        [[nodiscard]] constexpr auto& value() noexcept { return value_; };
        [[nodiscard]] constexpr auto& exception() noexcept { return exception_; };
    };

    handle_t h_;

private:
    bool full_ = false;

    constexpr void fill() {
        if (!full_) {
            h_();
            if (h_.promise().exception())
                std::rethrow_exception(h_.promise().exception());
            full_ = true;
        }
    }

public:
    co_generator_t(handle_t h) noexcept : h_(h) {}

    ~co_generator_t() noexcept { h_.destroy(); }

    constexpr explicit operator bool() {
        fill();
        return !h_.done();
    }

    [[nodiscard]] constexpr T operator()() {
        fill();
        full_ = false;
        return std::move(h_.promise().value());
    }
};
}
