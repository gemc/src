#pragma once

// Color conventions and logging macros
#include <gemc/guts/gutsConventions.h>
#include <gemc/goptions/goptions.h>
#include <gemc/glogging/gmessageCatalog.h>

// Geant4
#include "G4UIsession.hh"
#include <array>
#include <atomic>
#include <stdexcept>
#include <string>
#include <string_view>
#include <sstream>
#include <type_traits>
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
#define FUNCTION_NAME std::string(__FUNCSIG__) + std::string(" > ")
#else
#define FUNCTION_NAME std::string(__func__) + std::string(" > ")
#endif

/**
 * \def SFUNCTION_NAME
 * \brief Portable macro that expands to the current unqualified function name.
 *
 * This macro uses standard C++11 `__func__`, so for member functions it expands only to the
 * function name, not to `ClassName::functionName`.
 *
 * Use \ref FUNCTION_NAME "FUNCTION_NAME()" when compiler-specific qualified signatures are needed.
 *
 * \note The resulting string includes a trailing `" > "` so callers can concatenate it directly.
 */
#define SFUNCTION_NAME std::string(__func__) + std::string(" > ")

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
 * \brief Handles structured logging with verbosity and debug levels.
 *
 * GLogger centralizes formatted output for simulation components. Messages are emitted to the
 * Geant4 streams (`G4cout`/`G4cerr`) and can be filtered based on configuration obtained from
 * GOptions.
 *
 * Key responsibilities:
 * - Apply verbosity/debug filtering consistently across the codebase.
 * - Render standardized message headers including a per-instance atomic counter.
 * - Accept complete message templates with numbered, streamable parameters.
 * - Resolve cached translations automatically, retaining English for missing messages.
 * - Keep the transitional `*Template()` methods as forwarding aliases.
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
	 * \brief Constructs a GLogger instance and resolves its runtime configuration.
	 *
	 * The constructor queries the provided GOptions to resolve:
	 * - The verbosity level associated with \p lname.
	 * - The debug level associated with \p lname.
	 * - The message language and shared catalog configuration.
	 *
	 * It also emits a constructor-style debug message via
	 * \ref GLogger::debug "debug()" (only if debug is enabled for this logger name).
	 *
	 * \param gopts Shared pointer to the GOptions instance used for verbosity/debug lookup.
	 * \param cname Caller-provided class name (informational; currently not used for filtering).
	 * \param lname The logger name (subsystem identifier). This is used as the lookup key in
	 * GOptions.
	 */
	explicit GLogger(const std::shared_ptr<GOptions>& gopts, const std::string& cname,
	                 const std::string& lname = "")
		: class_name(cname), logger_name(lname), log_counter{0}, catalog(GMessageCatalog::load(gopts.get())) {
		// Resolve the runtime policy for this subsystem name from the global options.
		verbosity_level = gopts->getVerbosityFor(logger_name);
		debug_level     = gopts->getDebugFor(logger_name);

		// Optional lifetime trace to help identify logger ownership and construction order.
		debug(CONSTRUCTOR, "%1 logger", logger_name);
	}

	/**
	 * \brief Default constructor.
	 *
	 * This constructor leaves verbosity/debug settings at their defaults and resolves the language
	 * from environment or system settings. It is useful before GOptions configuration is available.
	 *
	 * \note If you use this constructor, ensure \c logger_name, \c verbosity_level, and \c debug_level
	 * are set through the normal construction path before relying on filtering behavior.
	 */
	GLogger() : catalog(GMessageCatalog::load()) {}

	/**
	 * \brief Destructor.
	 *
	 * Emits a destructor-style debug message via \ref GLogger::debug "debug()"
	 * (only if debug is enabled).
	 */
	~GLogger();

	/**
	 * \brief Logs a debug message if the debug level is nonzero.
	 *
	 * This method formats a complete message using the placeholder rules of info(), then prints
	 * the standardized header and optional constructor/destructor markers depending on \p type.
	 *
	 * \tparam Args Variadic template parameters representing any streamable types.
	 * \param type The debug message classification : NORMAL, CONSTRUCTOR, or DESTRUCTOR.
	 * \param message Complete message template.
	 * \param args Values referenced by the numbered placeholders.
	 *
	 * \note Debug output is suppressed entirely when \c debug_level == 0.
	 */
	template <typename... Args>
	void debug(debug_type type, std::string_view message, Args&&... args) const {
		if (debug_level == 0) return; // Debug disabled for this logger instance.
		const auto text = format_message(message, std::forward<Args>(args)...);

		switch (type) {
		case NORMAL:
			G4cout << guts::KCYN << header_string() << "DEBUG: " << text << guts::RST << G4endl;
			break;
		case CONSTRUCTOR:
			G4cout << guts::KCYN << header_string() << "DEBUG: " <<
				guts::CONSTRUCTORLOG << " " << text << " " << guts::CONSTRUCTORLOG << guts::RST << G4endl;
			break;
		case DESTRUCTOR:
			G4cout << guts::KCYN << header_string() << "DEBUG: " <<
				guts::DESTRUCTORLOG << " " << text << " " << guts::DESTRUCTORLOG << guts::RST << G4endl;
			break;
		}
	}

	/**
	 * \brief Logs a complete message template, optionally gated by verbosity level.
	 *
	 * Info messages support three levels of detail:
	 * - level 0 : Always printed (baseline information).
	 * - level 1 : Printed only when \c verbosity_level > 0.
	 * - level 2 : Printed only when \c verbosity_level > 1 (most detailed).
	 *
	 * Placeholders `%1` through `%99` identify arguments by their one-based position. They may be
	 * reordered or repeated; each supplied argument must appear at least once. `%%` emits a literal
	 * percent sign. Other percent signs are literal. Inserted values are never parsed as templates.
	 * Each argument is converted once using its own std::ostringstream and normal stream formatting.
	 * All logging categories share these placeholder rules. Catalog lookup precedes parameter substitution;
	 * untranslated messages retain their English templates. Language selection never changes numeric formatting.
	 *
	 * Filtering happens before formatting; C++ still evaluates argument expressions before the call.
	 * Dynamic or already-formatted text should be logged as a value: `info("%1", text)`.
	 *
	 * \tparam Args Parameter types with stream insertion support (at most 99 arguments).
	 * \param level The info importance/detail level (0, 1, or 2).
	 * \param message Complete message template.
	 * \param args Values referenced by the numbered placeholders.
	 * \throws std::invalid_argument If an emitted template references an unavailable/invalid argument
	 * or leaves a supplied argument unused. A suppressed message is not validated or formatted.
	 *
	 * \throws (process termination) If \p level is not 0, 1, or 2, this method prints a fatal error
	 * and terminates the process with \c goptions::EC_WRONG_VERBOSITY_LEVEL.
	 */
	template <typename... Args>
	void info(int level, std::string_view message, Args&&... args) const {
		if (info_enabled(level)) {
			const auto text = format_message(message, std::forward<Args>(args)...);
			G4cout << header_string() << "INFO L" << level << ": " << text << G4endl;
		}
	}

	/**
	 * \brief Convenience overload of \ref GLogger::info "info()" that defaults to level 0.
	 *
	 * This overload is intended for the most common informational messages that should always be
	 * printed irrespective of verbosity configuration.
	 *
	 * @tparam Args Variadic template parameters for any streamable types.
	 * \param message Complete message template.
	 * \param args Values referenced by the numbered placeholders.
	 */
	template <typename Message, typename... Args>
		requires std::is_convertible_v<Message, std::string_view>
	void info(Message&& message, Args&&... args) const {
		info(0, std::string_view(std::forward<Message>(message)), std::forward<Args>(args)...);
	}

	/**
	 * \brief Logs a warning message.
	 *
	 * Warning messages are always printed and are typically used for recoverable problems,
	 * suspicious conditions, or degraded behavior that does not warrant immediate termination.
	 *
	 * @tparam Args Variadic template parameters for any streamable types.
	 * \param message Complete message template, using the placeholder rules of info().
	 * \param args Values referenced by the numbered placeholders.
	 *
	 * @details This method does not consult verbosity or debug levels.
	 */
	template <typename... Args>
	void warning(std::string_view message, Args&&... args) const {
		const auto text = format_message(message, std::forward<Args>(args)...);
		G4cout << guts::KYEL << header_string() << guts::GWARNING << guts::KYEL << text
		       << guts::RST << G4endl;
	}

	/**
	 * \brief Logs an error message and terminates the process.
	 *
	 * This method prints:
	 * - The provided message.
	 * - The explicit exit code line.
	 *
	 * Then it terminates the process via \c std::exit(exit_code).
	 *
	 * @tparam Args Variadic template parameters for any streamable types.
	 * \param exit_code The program exit code to return to the calling environment.
	 * \param message Complete message template, using the placeholder rules of info().
	 * \param args Values referenced by the numbered placeholders.
	 *
	 * \note This function is marked \c [[noreturn]] because it always terminates the process.
	 */
	template <typename... Args>
	[[noreturn]] void error(int exit_code, std::string_view message, Args&&... args) const {
		const auto text = format_message(message, std::forward<Args>(args)...);
		G4cerr << guts::FATALERRORL << header_string() << guts::KRED << text << guts::RST << G4endl;
		G4cerr << guts::FATALERRORL << header_string() << guts::KRED << "Exit Code: " << exit_code << guts::RST
		       << G4endl;
		std::exit(exit_code);
	}

	/**
	 * \brief Logs a critical message.
	 *
	 * Critical messages are always printed and are intended for high-visibility output that
	 * should stand out (for example : major state transitions, run headers, or emphasized notices).
	 *
	 * @tparam Args Variadic template parameters for any streamable types.
	 * \param message Complete message template, using the placeholder rules of info().
	 * \param args Values referenced by the numbered placeholders.
	 */
	template <typename... Args>
	void critical(std::string_view message, Args&&... args) const {
		const auto text = format_message(message, std::forward<Args>(args)...);
		G4cout << guts::KBOLD << header_string() << guts::RST << text << G4endl;
	}

	/**
	 * \brief Transitional alias for info(), with identical template and filtering semantics.
	 * \param level Info verbosity level (0, 1, or 2).
	 * \param message Complete message template.
	 * \param args Values referenced by the numbered placeholders.
	 */
	template <typename... Args>
	void infoTemplate(int level, std::string_view message, Args&&... args) const {
		info(level, message, std::forward<Args>(args)...);
	}

	/// Logs a message template at info level 0; see infoTemplate(int, std::string_view, Args&&...).
	template <typename Message, typename... Args>
		requires std::is_convertible_v<Message, std::string_view>
	void infoTemplate(Message&& message, Args&&... args) const {
		info(0, std::string_view(std::forward<Message>(message)), std::forward<Args>(args)...);
	}

	/// Transitional alias for debug().
	template <typename... Args>
	void debugTemplate(debug_type type, std::string_view message, Args&&... args) const {
		debug(type, message, std::forward<Args>(args)...);
	}

	/// Transitional alias for warning().
	template <typename... Args>
	void warningTemplate(std::string_view message, Args&&... args) const {
		warning(message, std::forward<Args>(args)...);
	}

	/// Transitional alias for error().
	template <typename... Args>
	[[noreturn]] void errorTemplate(int exit_code, std::string_view message, Args&&... args) const {
		error(exit_code, message, std::forward<Args>(args)...);
	}

	/// Transitional alias for critical().
	template <typename... Args>
	void criticalTemplate(std::string_view message, Args&&... args) const {
		critical(message, std::forward<Args>(args)...);
	}

	/**
	 * \brief Returns the caller-provided class name associated with this logger instance.
	 *
	 * This value is currently informational and can be used by callers to include the originating
	 * class in their own message composition if desired.
	 *
	 * \return The stored class name string.
	 */
	[[nodiscard]] std::string get_class_name() const { return class_name; }

	/// Normalized language selected from options, environment, or system settings.
	[[nodiscard]] const std::string& get_language() const { return catalog->language(); }

private:
	std::string class_name; ///< Logical "owner" class name that instantiated the logger (informational).
	std::string logger_name;
	///< Logger/subsystem name used as the configuration lookup key in GOptions.
	int verbosity_level{}; ///< Resolved verbosity level (0 = minimal, higher = more detailed).
	int debug_level{};     ///< Resolved debug level (0 = off; nonzero enables debug printing).

	mutable std::atomic<int> log_counter{}; ///< Thread-safe per-instance message counter used by the header.
	std::shared_ptr<const GMessageCatalog> catalog; ///< Immutable translations shared by matching configurations.

	/// Validates and filters info levels before formatting their message templates.
	[[nodiscard]] bool info_enabled(int level) const {
		if (level != 0 && level != 1 && level != 2) {
			G4cerr << guts::FATALERRORL << header_string() << guts::GWARNING
			       << " Invalid verbosity level requested: " << level << guts::RST << G4endl;
			std::exit(goptions::EC_WRONG_VERBOSITY_LEVEL);
		}
		return level == 0 || (level == 1 && verbosity_level > 0) || (level == 2 && verbosity_level > 1);
	}

	/// Format a translated template without interpreting inserted parameter text.
	template <typename... Args>
	[[nodiscard]] std::string format_message(std::string_view message, Args&&... args) const {
		return catalog->format(message, std::forward<Args>(args)...);
	}

	/**
	 * \brief Constructs the standardized header prepended to each log message.
	 *
	 * The header includes the logger name and an incrementing counter:
	 * - The counter is incremented atomically on each call.
	 * - The returned header embeds the current counter value.
	 *
	 * \return A formatted header string in the form : \c "[ logger_name - counter ] ".
	 *
	 * \note This is a private helper; its behavior is documented here without cross-references.
	 */
	[[nodiscard]] std::string header_string() const {
		++log_counter; // Increment first so the first emitted message is "1" rather than "0".
		return " [ " + logger_name + " - " + std::to_string(log_counter.load()) + " ] ";
	}
};
