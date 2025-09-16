#pragma once

// 1) Prefer to see feature macros if available
#if defined(__has_include)
#  if __has_include(<version>)
#    include <version>
#  endif
#endif

#include <thread>

// 2) Use std::jthread when the library actually ships it
#if defined(__cpp_lib_jthread) && __cpp_lib_jthread >= 201911L
using jthread_alias = std::jthread;

#else
// Fallback: RAII "join-on-destruction" wrapper.
// Use composition (safer than inheriting from std::thread).
class jthread_alias {
	std::thread t_;
public:
	jthread_alias() noexcept = default;

	template<class F, class... Args>
	explicit jthread_alias(F&& f, Args&&... args)
		: t_(std::forward<F>(f), std::forward<Args>(args)...) {}

	jthread_alias(jthread_alias&&) noexcept            = default;
	jthread_alias& operator=(jthread_alias&&) noexcept = default;

	jthread_alias(const jthread_alias&)            = delete;
	jthread_alias& operator=(const jthread_alias&) = delete;

	~jthread_alias() { if (t_.joinable()) t_.join(); }

	// minimal forwarding API
	bool joinable() const noexcept { return t_.joinable(); }
	void join()                    { t_.join(); }
	void detach()                  { t_.detach(); }
	std::thread::id get_id() const noexcept { return t_.get_id(); }
	auto native_handle()                 { return t_.native_handle(); }
	void swap(jthread_alias& other) noexcept { t_.swap(other.t_); }
};
#endif
