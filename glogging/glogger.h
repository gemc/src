#pragma once

// Color conventions and logging macros
#include "gutsConventions.h"
#include "goptions.h"

// Geant4
#include "G4UIsession.hh"
#include <atomic>
#include <string>
#include <sstream>
#include <utility>


/**
 * \def FUNCTION_NAME
 * \brief Cross-platform macro that expands to a best-effort "pretty" function signature string.
 *
 * This macro is used to annotate messages with the current function signature, which can be useful
 * in diagnostics. The exact content is compiler-dependent:
 * - On Clang/GCC it uses `__PRETTY_FUNCTION__`
 * - On MSVC it uses `__FUNCSIG__`
 * - Otherwise it falls back to `__func__`
 *
 * \note The resulting string includes a trailing `" > "` so callers can concatenate it directly.
 */
#if defined(__clang__) || defined(__GNUC__)
#define FUNCTION_NAME std::string(__PRETTY_FUNCTION__) + std::string(" > ")
#elif defined(_MSC_VER)
#define FUNCTION_NAME   __FUNCSIG__ + std::string(" > ")
#else
#define FUNCTION_NAME   __func__ + std::string(" > ") // fallback
#endif

/**
 * \def SFUNCTION_NAME
 * \brief Portable macro that expands to the current function name (standard C++11 `__func__`).
 *
 * This version is intentionally minimal and stable across compilers, but it does not include
 * the full signature like \ref FUNCTION_NAME "FUNCTION_NAME()".
 *
 * \note The resulting string includes a trailing `" > "` so callers can concatenate it directly.
 */
#define SFUNCTION_NAME  __func__ + std::string(" > ")  // Always portable and standard since C++11


/**
 * \enum debug_type
 * \brief Classifies debug messages by intent.
 *
 * This enum allows the debug stream to distinguish between:
 * - Normal runtime debug output
 * - Constructor tracing output
 * - Destructor tracing output
 *
 * The constructor/destructor variants are typically used to visualize object lifetime and
 * ownership behavior in complex workflows.
 */
enum debug_type { NORMAL, CONSTRUCTOR, DESTRUCTOR };

/**
 * @class GLogger
 * @brief Handles structured logging with verbosity and debug levels.
 *
 * GLogger centralizes formatted output for simulation components. Messages are emitted to the
 * Geant4 streams (`G4cout`/`G4cerr`) and can be filtered based on configuration obtained from
 * GOptions.
 *
 * Key responsibilities:
 * - Apply verbosity/debug filtering consistently across the codebase.
 * - Render standardized message headers including a per-instance atomic counter.
 * - Provide convenience APIs that accept variadic "streamable" arguments, allowing callers to
 *   build messages without manual string formatting.
 *
 * \par Filtering model
 * - Debug messages are emitted only when the resolved debug level is nonzero.
 * - Info messages can be emitted at level 0, 1, or 2, each gated by the resolved verbosity level.
 * - Warning and critical messages are always emitted.
 * - Error messages are emitted and then the process terminates with an exit code.
 *
 * \warning This class writes to shared output streams. While the internal counter is thread-safe,
 * output interleaving is still possible depending on the underlying stream behavior.
 */
class GLogger
{
public:
	/**
	 * @brief Constructs a GLogger instance and resolves its runtime configuration.
	 *
	 * The constructor queries the provided GOptions to resolve:
	 * - The verbosity level associated with `lname`
	 * - The debug level associated with `lname`
	 *
	 * It also emits a constructor-style debug message via
	 * \ref GLogger::debug "debug()" (only if debug is enabled).
	 *
	 * @param gopts Shared pointer to the GOptions instance used for verbosity/debug lookup.
	 * @param cname Caller-provided class name (informational; currently not used for filtering).
	 * @param lname The logger name (subsystem identifier). This is used as the lookup key in
	 * GOptions.
	 */
	explicit GLogger(const std::shared_ptr<GOptions>& gopts, const std::string& cname, const std::string& lname = "")
		: class_name(cname), logger_name(lname), log_counter{0} {
		verbosity_level = gopts->getVerbosityFor(logger_name);
		debug_level     = gopts->getDebugFor(logger_name);
		debug(CONSTRUCTOR, logger_name, " logger");
	}

	/**
	 * @brief Default constructor.
	 *
	 * This constructor does not resolve verbosity/debug settings and leaves the logger in a
	 * default-initialized state. It is primarily useful for scenarios where a logger instance
	 * must exist before configuration is available.
	 *
	 * \note If you use this constructor, ensure `logger_name`, `verbosity_level`, and `debug_level`
	 * are set through the normal construction path before relying on filtering behavior.
	 */
	GLogger() = default;

	/**
	 * @brief Destructor.
	 *
	 * Emits a destructor-style debug message via \ref GLogger::debug "debug()"
	 * (only if debug is enabled).
	 */
	~GLogger() { debug(DESTRUCTOR, logger_name, " logger"); }

	/**
	 * \brief Logs a debug message if the debug level is nonzero.
	 *
	 * This method builds a single message string from `args...` using stream insertion into an
	 * `std::ostringstream`. It then prints the message with a standardized header and optional
	 * constructor/destructor markers depending on \p type.
	 *
	 * \tparam Args Variadic template parameters representing any streamable types.
	 * \param type The debug message classification : NORMAL, CONSTRUCTOR, or DESTRUCTOR.
	 * \param args The message components. Each argument must be insertable into `std::ostream`.
	 *
	 * @details Implementation techniques used:
	 * - Variadic templates for a flexible number of message components.
	 * - Perfect forwarding (`std::forward`) to preserve value categories.
	 * - Fold expression `(oss << ... << arg)` to stream all arguments.
	 *
	 * \note Debug output is suppressed entirely when `debug_level == 0`.
	 */
	template <typename... Args>
	void debug(debug_type type, Args&&... args) const {
		if (debug_level == 0) return;

		std::ostringstream oss;
		(oss << ... << std::forward<Args>(args));

		switch (type) {
		case NORMAL:
			G4cout << KCYN << header_string() << "DEBUG: " << oss.str() << RST << G4endl;
			break;
		case CONSTRUCTOR:
			G4cout << KCYN << header_string() << "DEBUG: " <<
				CONSTRUCTORLOG << " " << oss.str() << " " << CONSTRUCTORLOG << RST << G4endl;
			break;
		case DESTRUCTOR:
			G4cout << KCYN << header_string() << "DEBUG: " <<
				DESTRUCTORLOG << " " << oss.str() << " " << DESTRUCTORLOG << RST << G4endl;
			break;
		}
	}

	/**
	 * \brief Logs an informational message, optionally gated by verbosity level.
	 *
	 * Info messages support three levels of detail:
	 * - level 0 : Always printed (baseline information).
	 * - level 1 : Printed only when `verbosity_level > 0`.
	 * - level 2 : Printed only when `verbosity_level > 1` (most detailed).
	 *
	 * \tparam Args Variadic template parameters for any streamable types.
	 * \param level The info importance/detail level (0, 1, or 2).
	 * \param args Message components to log. Each argument must be stream-insertable.
	 *
	 * \throws (process termination) If \p level is not 0, 1, or 2, this method prints a fatal error
	 * and terminates the process with `EC_WRONG_VERBOSITY_LEVEL`.
	 */
	template <typename... Args>
	void info(int level, Args&&... args) const {
		// error if the level is not 0, 1 or 2
		if (level != 0 && level != 1 && level != 2) {
			G4cerr << FATALERRORL << header_string() << GWARNING << " Invalid verbosity level requested: " << level <<
				RST << G4endl;
			exit(EC_WRONG_VERBOSITY_LEVEL);
		}

		if (level == 0 || (level == 1 && verbosity_level > 0) || (level == 2 && verbosity_level > 1)) {
			std::ostringstream oss;
			(oss << ... << std::forward<Args>(args));
			G4cout << header_string() << "INFO L" << level << ": " << oss.str() << G4endl;
		}
	}

	/**
	 * @brief Convenience overload of \ref GLogger::info "info()" that defaults to level 0.
	 *
	 * This overload is intended for the most common informational messages that should always be
	 * printed irrespective of verbosity configuration.
	 *
	 * @tparam Args Variadic template parameters for any streamable types.
	 * @param args Streamable message components.
	 */
	template <typename... Args>
	void info(Args&&... args) const { info(0, std::forward<Args>(args)...); }

	/**
	 * @brief Logs a warning message.
	 *
	 * Warning messages are always printed and are typically used for recoverable problems,
	 * suspicious conditions, or degraded behavior that does not warrant immediate termination.
	 *
	 * @tparam Args Variadic template parameters for any streamable types.
	 * @param args Message parts to be logged.
	 *
	 * @details This method does not consult verbosity or debug levels.
	 */
	template <typename... Args>
	void warning(Args&&... args) const {
		std::ostringstream oss;
		(oss << ... << std::forward<Args>(args));
		G4cout << KYEL << header_string() << GWARNING << KYEL << oss.str() << RST << G4endl;
	}

	/**
	 * @brief Logs an error message and terminates the process.
	 *
	 * This method prints:
	 * - The provided message
	 * - The explicit exit code line
	 *
	 * Then it terminates the process via `std::exit(exit_code)`.
	 *
	 * @tparam Args Variadic template parameters for any streamable types.
	 * @param exit_code The program exit code to return to the calling environment.
	 * @param args Message parts to be logged before exiting.
	 *
	 * @note This function is marked `[[noreturn]]` because it always terminates the process.
	 */
	template <typename... Args>
	[[noreturn]] void error(int exit_code, Args&&... args) const {
		std::ostringstream oss;
		(oss << ... << std::forward<Args>(args));
		G4cerr << FATALERRORL << header_string() << KRED << oss.str() << RST << G4endl;
		G4cerr << FATALERRORL << header_string() << KRED << "Exit Code: " << exit_code << RST << G4endl;
		std::exit(exit_code);
	}

	/**
	 * @brief Logs a critical message.
	 *
	 * Critical messages are always printed and are intended for high-visibility output that
	 * should stand out (for example : major state transitions, run headers, or emphasized notices).
	 *
	 * @tparam Args Variadic template parameters for any streamable types.
	 * @param args Message parts to be logged.
	 */
	template <typename... Args>
	void critical(Args&&... args) const {
		std::ostringstream oss;
		(oss << ... << std::forward<Args>(args));
		G4cout << KBOLD << header_string() << RST << oss.str() << G4endl;
	}

	/**
	 * @brief Returns the caller-provided class name associated with this logger instance.
	 *
	 * This value is currently informational and can be used by callers to include the originating
	 * class in their own message composition if desired.
	 *
	 * @return The stored class name string.
	 */
	[[nodiscard]] std::string get_class_name() const { return class_name; }

private:
	std::string class_name; ///< Logical "owner" class name that instantiated the logger (informational).
	std::string logger_name;
	///< Logger/subsystem name used as the configuration lookup key in \ref GOptions "GOptions".
	int verbosity_level{}; ///< Resolved verbosity level for this logger instance (0 = minimal, higher = more detailed).
	int debug_level{};     ///< Resolved debug level for this logger instance (0 = off; nonzero enables debug printing).

	mutable std::atomic<int> log_counter{}; ///< Thread-safe per-instance message counter used by the message header.

	/**
	 * \brief Constructs the standardized header prepended to each log message.
	 *
	 * The header includes the logger name and an incrementing counter:
	 * - The counter is incremented atomically on each call.
	 * - The returned header embeds the current counter value.
	 *
	 * \return A formatted header string in the form : `[ logger_name - counter ]`.
	 *
	 * \note This is a private helper; its behavior is documented here without external cross-references.
	 */
	[[nodiscard]] std::string header_string() const {
		++log_counter;
		return " [ " + logger_name + " - " + std::to_string(log_counter.load()) + " ] ";
	}
};
