#pragma once

/**
 * @file gbase.h
 * @brief Lightweight CRTP base class that provides a pre-configured logger to derived types.
 *
 * This header defines:
 * - A small utility function, demangle(), used to turn compiler-specific type names
 *   (typically produced by \c typeid(T).name()) into more readable strings when supported.
 * - The template class \ref GBase , which centralizes logger construction and lifetime
 *   management for derived classes using the CRTP (Curiously Recurring Template Pattern).
 *
 * Design goals:
 * - Make logging setup consistent across many derived classes.
 * - Keep logger ownership explicit and predictable via \c std::shared_ptr.
 * - Optionally allow sharing a single logger across multiple objects to reduce overhead.
 *
 * Notes on demangling:
 * - When compiled with \c __GNUG__ (GNU-compatible toolchains), demangling uses
 *   \c abi::__cxa_demangle from \c <cxxabi.h>.
 * - On other toolchains, demangle() returns the input name unchanged.
 */

// C++
#include <iostream>
#include <string>
#include <typeinfo>

// gbase
#include "glogger.h"

#if defined(__GNUG__)
#include <cxxabi.h>
#include <memory>

/**
 * @brief Convert a mangled C++ type name into a human-readable form when available.
 *
 * This helper is typically used with \c typeid(T).name() to make log messages and
 * diagnostics easier to understand.
 *
 * Implementation details:
 * - Uses \c abi::__cxa_demangle (GNU-compatible compilers).
 * - Allocates a demangled C string that must be freed; this function wraps the pointer
 *   in a \c std::unique_ptr with \c std::free as deleter to ensure exception safety.
 *
 * Error handling:
 * - If demangling fails (non-zero status) or the returned pointer is null, the function
 *   returns the original \p name unchanged.
 *
 * Performance notes:
 * - Demangling may allocate and is not intended for tight inner loops; it is typically
 *   used for initialization/log headers and occasional diagnostics.
 *
 * @param name A mangled type name, usually produced by \c typeid(T).name().
 * @return A demangled string if demangling succeeds, otherwise the original \p name.
 */
std::string inline demangle(const char* name) {
	int status      = 0;
	using deleter_t = void(*)(void*);
	std::unique_ptr<char, deleter_t> demangled(abi::__cxa_demangle(name, nullptr, nullptr, &status), std::free);
	return (status == 0 && demangled) ? demangled.get() : name;
}

#else
/**
 * @brief Fallback demangle implementation for non-GNU toolchains.
 *
 * On compilers/platforms where GNU-style demangling is not available, this function
 * simply returns the input string unchanged.
 *
 * This is a deliberate portability choice: the calling code can rely on the function
 * existing everywhere, while still benefiting from readable names where supported.
 *
 * @param name A mangled type name, usually produced by \c typeid(T).name().
 * @return The original \p name (no demangling performed).
 */
std::string demangle(const char* name) {
	return name; // No demangling on non-GNU compilers
}
#endif


/**
 * @brief CRTP base class that provides logging facilities to the derived class.
 *
 * The template parameter \c Derived is the concrete type inheriting from this base:
 * \code
 * class MyType : public GBase<MyType> { ... };
 * \endcode
 *
 * Responsibilities:
 * - Construct and hold a \c std::shared_ptr<GLogger> for the derived object.
 * - Emit standard lifecycle log messages (constructor/destructor) when a logger exists.
 * - Support two usage modes:
 *   1) Create a dedicated logger from \c GOptions for each derived object.
 *   2) Reuse an externally managed logger (shared logger) across multiple objects.
 *
 * Ownership model:
 * - The logger is stored as a \c std::shared_ptr so it can be shared safely and cheaply.
 * - Copying a \ref GBase performs a shallow copy of the logger pointer (shared ownership).
 *
 * Threading notes:
 * - This class does not add synchronization; it only stores and forwards a logger pointer.
 * - Whether logging is thread-safe depends on the  GLogger implementation and its sinks.
 *
 * Usage guidance:
 * - Prefer the \c GOptions-based constructor for most components.
 * - Prefer the shared-logger constructor when constructing many short-lived objects, or when
 *   logger creation/configuration is known to be expensive for a particular subsystem.
 *
 * @tparam Derived The concrete class inheriting from this base (CRTP pattern).
 */
template <typename Derived>
class GBase
{
public:
	/**
	 * @brief Construct a base that creates (and owns) a logger for the derived instance.
	 *
	 * This constructor:
	 * - Creates a  GLogger using the provided \c GOptions.
	 * - Uses the derived type name (via compiler RTTI) as the logical component name.
	 * - Optionally accepts an additional \p logger_name used to select or label the logger
	 *   instance according to the  GLogger / \c GOptions conventions.
	 * - Emits a constructor log message through the newly created logger.
	 *
	 * Expected invariants after construction:
	 * - The protected member \c log is non-null (unless  GLogger construction throws).
	 *
	 * @param gopt Shared configuration/options used to initialize  GLogger.
	 * @param logger_name Optional logger identifier or channel name (may be empty).
	 */
	explicit GBase(const std::shared_ptr<GOptions>& gopt, std::string logger_name = "") {
		log = std::make_shared<GLogger>(gopt, getDerivedName(), logger_name);
		log->debug(CONSTRUCTOR, getDerivedName());
	}

	// sharing logger for heavy creations like gtouchable
	/**
	 * @brief Construct a base that reuses an existing logger.
	 *
	 * This is intended for cases where logger construction is expensive or where a group
	 * of objects should share identical logging configuration and output destination.
	 *
	 * The provided logger is stored as-is (shared ownership via \c std::shared_ptr).
	 * This constructor does not emit a constructor log message because it does not create
	 * or configure the logger; the calling code owns that responsibility.
	 *
	 * Important:
	 * - Passing a null \p logger is permitted by the type system; if null is passed, the
	 *   derived object simply has no logger and lifecycle logging is suppressed.
	 *
	 * @param logger Shared logger instance to be reused by this object.
	 */
	explicit GBase(const std::shared_ptr<GLogger>& logger) : log(logger) {
	}

	/**
	 * @brief Destructor that logs object destruction when a logger is available.
	 *
	 * Behavior:
	 * - If \c log is non-null, emits a standard destructor message.
	 * - If \c log is null, performs no logging.
	 *
	 * Rationale:
	 * - The destructor must be safe in all shutdown orders; it therefore checks \c log
	 *   before attempting to log.
	 */
	virtual ~GBase() { if (log) log->debug(DESTRUCTOR, getDerivedName()); }

	// Important: because we declared a destructor, default these explicitly.
	/**
	 * @brief Copy constructor (shallow copy of the logger pointer).
	 *
	 * Copying a \ref GBase copies the \c std::shared_ptr<GLogger>, meaning both objects will
	 * refer to the same logger instance.
	 *
	 * Implications:
	 * - This is usually desirable: copies of an object retain the same logging channel/sinks.
	 * - If a derived type requires independent loggers per instance, it should avoid copying
	 *   or provide a custom copy policy at the derived level.
	 */
	GBase(const GBase&) = default; // shallow copy of shared_ptr

	/**
	 * @brief Copy assignment (shallow copy of the logger pointer).
	 *
	 * After assignment, both objects will refer to the same logger instance.
	 * This operation does not emit any log messages.
	 */
	GBase& operator=(const GBase&) = default;

	/**
	 * @brief Move constructor.
	 *
	 * Moves the internal \c std::shared_ptr<GLogger> from the source object.
	 * Marked \c noexcept to preserve move semantics in standard containers.
	 *
	 * After the move:
	 * - The moved-to object receives the logger pointer.
	 * - The moved-from object's \c log becomes null or remains in a valid, unspecified state
	 *   per standard \c std::shared_ptr move semantics.
	 */
	GBase(GBase&&) noexcept = default; // otherwise move would be suppressed

	/**
	 * @brief Move assignment operator.
	 *
	 * Moves the internal \c std::shared_ptr<GLogger> from the source object.
	 * Marked \c noexcept to preserve move semantics in standard containers.
	 *
	 * This operation does not emit any log messages.
	 */
	GBase& operator=(GBase&&) noexcept = default;

private:
	/**
	 * @brief Obtain a readable name for the derived type.
	 *
	 * The returned value is used for logger initialization and lifecycle messages.
	 * On GNU-compatible toolchains, the type name may be demangled for readability.
	 *
	 * This function is intentionally private because:
	 * - It is an implementation detail of the logging/lifecycle policy.
	 * - Derived classes should treat the component name as an internal logging label, not
	 *   as part of their public API contract.
	 *
	 * @return A readable derived type name suitable for logs and diagnostics.
	 */
	[[nodiscard]] std::string getDerivedName() const { return demangle(typeid(Derived).name()); }

protected:
	/**
	 * @brief Shared logger used by the derived class for emitting messages.
	 *
	 * This member is protected so derived classes can log with minimal ceremony.
	 *
	 * Typical usage in derived classes:
	 * - \c log->info(...)
	 * - \c log->debug(...)
	 * - \c log->warning(...)
	 *
	 * Depending on how the base is constructed, this logger may be:
	 * - A dedicated logger created from \c GOptions, or
	 * - A shared logger passed in from external code.
	 *
	 * Nullability:
	 * - When constructed with the \c GOptions-based constructor, this is expected to be non-null.
	 * - When constructed with the shared-logger constructor, this is whatever was provided
	 *   (including the possibility of null).
	 *
	 * Lifetime:
	 * - This is a \c std::shared_ptr; the logger remains alive as long as any owner retains it.
	 */
	std::shared_ptr<GLogger> log;
};
