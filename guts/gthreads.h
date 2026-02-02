#pragma once

// 1) Prefer to see feature macros if available
#if defined(__has_include)
#  if __has_include(<version>)
#    include <version>
#  endif
#endif

#include <thread>

/**
 * @file gthreads.h
 * @brief Small compatibility wrapper providing a jthread-like type.
 *
 * This header defines \c jthread_alias, which maps to:
 * - \c std::jthread when the standard library provides it (C++20 and library support), or
 * - a minimal RAII wrapper around \c std::thread that joins on destruction otherwise.
 *
 * The fallback is intentionally small: it provides only the subset of functionality used by the project
 * (construction, move support, join/detach, id/native handle access, and swap).
 */

// 2) Use std::jthread when the library actually ships it
#if defined(__cpp_lib_jthread) && __cpp_lib_jthread >= 201911L

/**
 * @typedef jthread_alias
 * @brief Alias to \c std::jthread when available.
 *
 * When \c std::jthread exists, it provides automatic joining and cooperative cancellation
 * (via stop tokens) in the standard way.
 */
using jthread_alias = std::jthread;

#else

/**
 * @class jthread_alias
 * @brief Fallback RAII "join-on-destruction" thread wrapper.
 *
 * This class is used when \c std::jthread is not available in the standard library.
 *
 * Key properties:
 * - Owns a \c std::thread instance via composition (not inheritance).
 * - Joins the thread in the destructor if it is still joinable.
 * - Non-copyable; movable.
 *
 * Differences vs \c std::jthread:
 * - No stop-token / cooperative cancellation support.
 * - Only the minimal forwarding API needed by the codebase is exposed.
 */
class jthread_alias
{
	std::thread t_;

public:
	/**
	 * @brief Construct an empty (non-joinable) wrapper.
	 */
	jthread_alias() noexcept = default;

	/**
	 * @brief Start a new thread by forwarding arguments to the underlying \c std::thread.
	 *
	 * @tparam F Callable type.
	 * @tparam Args Argument pack forwarded to @p f.
	 * @param f Callable to run in the new thread.
	 * @param args Arguments passed to @p f.
	 */
	template <class F, class... Args>
	explicit jthread_alias(F&& f, Args&&... args)
		: t_(std::forward<F>(f), std::forward<Args>(args)...) {
	}

	jthread_alias(jthread_alias&&) noexcept            = default;
	jthread_alias& operator=(jthread_alias&&) noexcept = default;

	jthread_alias(const jthread_alias&)            = delete;
	jthread_alias& operator=(const jthread_alias&) = delete;

	/**
	 * @brief Join the underlying thread on destruction if still joinable.
	 */
	~jthread_alias() { if (t_.joinable()) t_.join(); }

	/**
	 * @brief Check whether the underlying thread can be joined.
	 * @return @c true if joinable, @c false otherwise.
	 */
	bool joinable() const noexcept { return t_.joinable(); }

	/**
	 * @brief Join the underlying thread.
	 *
	 * @warning Calling this when not joinable has the same preconditions as \c std::thread::join.
	 */
	void join() { t_.join(); }

	/**
	 * @brief Detach the underlying thread.
	 *
	 * After detaching, the wrapper no longer represents a joinable thread.
	 */
	void detach() { t_.detach(); }

	/**
	 * @brief Get the underlying thread id.
	 * @return The \c std::thread::id of the owned thread.
	 */
	std::thread::id get_id() const noexcept { return t_.get_id(); }

	/**
	 * @brief Access the native handle of the underlying thread.
	 * @return Native handle as returned by \c std::thread::native_handle.
	 */
	auto native_handle() { return t_.native_handle(); }

	/**
	 * @brief Swap the underlying thread with another wrapper.
	 * @param other Wrapper to swap with.
	 */
	void swap(jthread_alias& other) noexcept { t_.swap(other.t_); }
};
#endif
