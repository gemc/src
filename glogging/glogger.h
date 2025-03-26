#ifndef GLOGGER_H
#define GLOGGER_H

// Color conventions and logging macros
#include "gutsConventions.h"
#include "goptions.h"

// Geant4
#include "G4UIsession.hh"
#include <atomic>
#include <string>
#include <cstdlib>
#include <sstream>
#include <utility>


enum debug_type { NORMAL, CONSTRUCTOR, DESTRUCTOR };

/**
 * @class GLogger
 * @brief Handles structured logging with verbosity and debug levels.
 *
 * GLogger manages output messages categorized as debug, info, warning, error, and critical.
 * It allows conditional printing based on verbosity and debug flags, supports colored output,
 * and automatically adds a counter to each message.
 */
class GLogger {
public:
	/**
	 * @brief Constructs a GLogger instance.
	 * @param gopts Pointer to GOptions instance used for verbosity/debug lookup.
	 * @param name The verbosity or debyg name is a string used to identify the logger and as header for all messages
	 */
	explicit GLogger(GOptions* gopts, std::string vname)
			: name(std::move(vname)), log_counter(0) {
		verbosity_level = gopts->getVerbosityFor(name);
		debug_level = gopts->getDebugFor(name);
	}

	// default constructor
	GLogger() = default;

	~GLogger() = default;

	/**
	 * @brief Logs a debug message, if debug level is set.
	 *
	 * @tparam Args Variadic template parameters, representing any streamable types.
	 * @param type The debug type (1 = normal, 10 = constructor, -10 = destructor).
	 * @param args Arguments to be forwarded and streamed into the log message.
	 *
	 * @details This method uses:
	 * - **Variadic templates** to accept any number of arguments.
	 * - **Perfect forwarding** with `std::forward` to preserve value categories.
	 * - **Fold expressions** to stream all arguments into an `ostringstream`.
	 */
	template <typename... Args>
	void debug(debug_type type, Args&&... args) const {
		if (debug_level == 0) return;

		std::ostringstream oss;
		(oss << ... << std::forward<Args>(args));

		switch (type) {
			case NORMAL:
				G4cout << KYEL << header_string() << oss.str() << RST << G4endl;
				break;
			case CONSTRUCTOR:
				G4cout << KCYN << header_string() << CONSTRUCTORLOG << " constructor " << CONSTRUCTORLOG << " " << RST << oss.str() << G4endl;
				break;
			case DESTRUCTOR:
				G4cout << KCYN << header_string() << DESTRUCTORLOG << " destructor " << DESTRUCTORLOG << " " << RST << oss.str() << G4endl;
				break;
		}

	}

	/**
	 * @brief Logs an info message, conditionally based on verbosity level.
	 *
	 * @tparam Args Variadic template parameters for any streamable types.
	 * @param level The importance level (0 = always, 1 = only if verbosity > 0).
	 * @param args Streamable message components.
	 *
	 * @details Uses fold expressions and perfect forwarding to build the message.
	 */
	template <typename... Args>
	void info(int level, Args&&... args) const {
		if (level == 0 || (level == 1 && verbosity_level > 0)) {
			std::ostringstream oss;
			(oss << ... << std::forward<Args>(args));
			G4cout << header_string() << oss.str() << G4endl;
		}
	}

	/**
	 * @brief Overloaded version of info() with default level = 0.
	 *
	 * @tparam Args Variadic template parameters for any streamable types.
	 * @param args Streamable message components.
	 */
	template <typename... Args>
	void info(Args&&... args) const {
		info(0, std::forward<Args>(args)...);
	}

	/**
	 * @brief Logs a warning message.
	 *
	 * @tparam Args Variadic template parameters for any streamable types.
	 * @param args Message parts to be logged.
	 *
	 * @details Always prints regardless of verbosity or debug levels.
	 */
	template <typename... Args>
	void warning(Args&&... args) const {
		std::ostringstream oss;
		(oss << ... << std::forward<Args>(args));
		G4cout << KYEL << header_string() << GWARNING << oss.str() << RST << G4endl;
	}

	/**
	 * @brief Logs an error message and exits the application.
	 *
	 * @tparam Args Variadic template parameters for any streamable types.
	 * @param exit_code The program exit code.
	 * @param args Message parts to be logged before exiting.
	 *
	 * @note This function is marked [[noreturn]] because it always terminates the process.
	 */
	template <typename... Args>
	[[noreturn]] void error(int exit_code, Args&&... args) const {
		std::ostringstream oss;
		(oss << ... << std::forward<Args>(args));
		G4cerr << FATALERRORL << header_string() << GWARNING << oss.str() << RST << G4endl;
		std::exit(exit_code);
	}

	/**
	 * @brief Logs a critical message. Always printed.
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

private:
	std::string name;       ///< Prefix for all messages
	int verbosity_level;          ///< Verbosity level (0 = low, >0 = detailed)
	int debug_level;              ///< Debug level: 0 = off, 1 = normal, 10/-10 = ctor/dtor

	mutable std::atomic<int> log_counter; ///< Thread-safe counter for messages

	/**
	 * @brief Constructs a formatted log header with a running counter.
	 * @return A unique string prepended to each log message.
	 *
	 * @note Uses atomic increment to ensure thread safety.
	 */
	[[nodiscard]] std::string header_string() const {
		log_counter++;
		return name + " [" + std::to_string(log_counter.load()) + "] ";
	}
};

#endif // GLOGGER_H
