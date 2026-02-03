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
 * The fallback is intentionally small and conservative: it provides only the subset of functionality
 * used by the project. The intent is to avoid sprinkling conditional compilation throughout the
 * codebase while still getting safe thread joining semantics.
 *
 * Key contract for the fallback:
 * - If a thread is started and still joinable at destruction, it is joined.
 * - The wrapper is non-copyable and movable.
 * - The API mirrors the \c std::thread subset that is needed (join, detach, id, native handle, swap).
 */

// 2) Use std::jthread when the library actually ships it
#if defined(__cpp_lib_jthread) && __cpp_lib_jthread >= 201911L

/**
 * @typedef jthread_alias
 * @brief Alias to \c std::jthread when available.
 *
 * When \c std::jthread exists, it provides automatic joining and cooperative cancellation
 * (via stop tokens) in the standard way.
 *
 * @note The rest of the codebase should prefer \c jthread_alias so the implementation choice
 *       remains centralized in this header.
 */
using jthread_alias = std::jthread;

#else

/**
 * @class jthread_alias
 * @brief Fallback RAII "join-on-destruction" thread wrapper.
 *
 * This class is used when \c std::jthread is not available in the standard library.
 *
 * Ownership model:
 * - The object *owns* the underlying \c std::thread instance.
 * - If that thread is joinable at destruction, the destructor joins it.
 *
 * Differences vs \c std::jthread:
 * - No stop-token / cooperative cancellation support.
 * - Only a minimal subset of the thread API is exposed.
 *
 * Typical usage:
 * @code{.cpp}
 * jthread_alias t([]{ do_work(); });
 * // ... later ...
 * // joins automatically on scope exit if still joinable
 * @endcode
 *
 * @warning The destructor joins unconditionally when joinable. This is a safety feature, but
 *          it also means a long-running thread may delay scope exit. Ensure the thread function
 *          has clear termination conditions.
 */
class jthread_alias
{
	/**
	 * @brief Owned thread instance.
	 *
	 * Invariant: \c t_ is either default-constructed (not joinable), moved-from (not joinable),
	 * or represents a running/finished thread that may be joinable.
	 */
	std::thread t_;

public:
	/**
	 * @brief Construct an empty (non-joinable) wrapper.
	 *
	 * After default construction, \ref jthread_alias::joinable "joinable()" returns @c false.
	 */
	jthread_alias() noexcept = default;

	/**
	 * @brief Start a new thread by forwarding arguments to the underlying \c std::thread.
	 *
	 * @tparam F Callable type.
	 * @tparam Args Argument pack forwarded to @p f.
	 * @param f Callable to run in the new thread.
	 * @param args Arguments passed to @p f.
	 *
	 * @note This constructor is \c explicit to avoid accidental implicit thread starts.
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
	 *
	 * This mirrors the "safe by default" behavior typically sought with \c std::jthread.
	 *
	 * @warning If the owned thread function can block indefinitely, destruction will also block.
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
	 * Preconditions match \c std::thread::join.
	 *
	 * @warning Calling this when not joinable has undefined behavior per \c std::thread rules
	 *          (typically terminating the program).
	 */
	void join() { t_.join(); }

	/**
	 * @brief Detach the underlying thread.
	 *
	 * After detaching, the wrapper no longer represents a joinable thread, and the destructor
	 * will not join.
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
	 *
	 * @note This is provided for integration with low-level platform APIs when needed.
	 */
	auto native_handle() { return t_.native_handle(); }

	/**
	 * @brief Swap the underlying thread with another wrapper.
	 * @param other Wrapper to swap with.
	 *
	 * After swapping, each wrapper owns the other's prior thread.
	 */
	void swap(jthread_alias& other) noexcept { t_.swap(other.t_); }
};
#endif
