/**
 *
 * \mainpage GLogger Module
 *
 * \tableofcontents
 *
 * \section intro_sec Introduction
 * The GLogger module provides structured, consistently formatted logging for the simulation runtime.
 * It is intended to be the single place where verbosity/debug policies and log formatting rules are
 * applied so that the rest of the codebase can emit messages without re-implementing filtering,
 * headers, or formatting.
 *
 * At a high level, a logger instance:
 * - Looks up its verbosity and debug configuration in GOptions using a caller-chosen subsystem name.
 * - Prepends each line with a compact header that identifies the subsystem and a monotonically
 *   increasing per-instance message counter.
 * - Emits messages to Geant4 output streams (i.e. \c G4cout / \c G4cerr).
 *
 * \section categories_sec Message categories
 * The logger supports multiple message categories:
 * - **Debug** messages (filtered by a debug level)
 * - **Informational** messages (filtered by a verbosity level)
 * - **Warnings** (always printed)
 * - **Errors** (printed and then the process terminates with an exit code)
 * - **Critical** messages (always printed with emphasized formatting)
 *
 * \section verbosity_sec Verbosity and debug levels
 * Many subsystems in this project derive from GLogger (or aggregate a GLogger instance) and expose
 * runtime-configurable output control.
 *
 * Verbosity affects *informational* messages:
 * - **Level 0** prints baseline run information (always shown).
 * - **Level 1** prints additional operational details (shown when verbosity > 0).
 * - **Level 2** prints the most detailed step-by-step information (shown when verbosity > 1).
 *
 * Debug affects *debug* messages:
 * - When debug is enabled (nonzero), debug messages are printed, including optional constructor /
 *   destructor markers that help trace object lifetime and ownership transitions.
 * - Debug output is intended for developers and may be verbose and implementation-specific.
 *
 * @section options_sec Available Options and their usage
 *
 * This module reads the following option keys from the runtime option provider:
 *
 * - `verbosity`
 *   - Type: structured option (sequence/map, implementation-defined by \ref GOptions)
 *   - Meaning: per-logger verbosity configuration
 *   - Behavior:
 *     - loggers query their verbosity level using their configured logger name as a key
 *     - typical levels:
 *       - `0` prints baseline informational output
 *       - `1` prints additional informational output
 *       - `2` prints the most detailed informational output
 *
 * - `debug`
 *   - Type: structured option (sequence/map, implementation-defined by \ref GOptions)
 *   - Meaning: per-logger debug enablement / debug level configuration
 *   - Behavior:
 *     - loggers query their debug configuration using their configured logger name as a key
 *     - values are commonly `false`/`true` (mapped to 0/1) or an integer level, depending on consumer expectations
 *
 * Note: \ref GOptions::GOptions "GOptions(argc,argv,...)" defines these keys as global conventions so any module
 * can participate in consistent logging without re-defining these options.
 *
 * \section header_sec Message header format
 * Each emitted message is prepended with a compact header that includes:
 * - The configured logger name (a logical subsystem identifier).
 * - A monotonically increasing counter (per logger instance).
 *
 * The counter is particularly useful in multi-threaded environments because it helps correlate
 * message order even when different threads interleave output.
 *
 * \section details_sec Implementation notes
 * - The implementation is intentionally lightweight and header-driven.
 * - Debug/info/warning/error/critical accept a variadic list of "streamable" arguments, which are
 *   concatenated using an \c std::ostringstream.
 * - A \c std::atomic<int> counter is used to produce the per-instance sequence number in a
 *   thread-safe manner.
 * - Debug logging supports special constructor/destructor styles to make lifetime tracing easier.
 *
 * \section usage_sec Usage
 * 1. Construct a \ref GLogger "GLogger" instance with :
 *    - A shared pointer to GOptions (used to resolve verbosity/debug configuration).
 *    - A caller class name (informational; currently not used for filtering).
 *    - A logger name (used as a lookup key in GOptions).
 * 2. Call the relevant message methods:
 *    - \ref GLogger::debug "debug()"
 *    - \ref GLogger::info "info()"
 *    - \ref GLogger::warning "warning()"
 *    - \ref GLogger::error "error()"
 *    - \ref GLogger::critical "critical()"
 *
 * \note The \ref GLogger::error "error()" method is marked \c [[noreturn]] and terminates the process.
 *
 * \section examples_sec Examples
 *
 * \subsection ex_basic_sec Example: basic logger construction and baseline info
 * This example shows how a subsystem creates a logger with a configured subsystem name and emits
 * always-on informational output.
 *
 * \code
 * // Suppose gopts is a std::shared_ptr<GOptions> configured elsewhere.
 * GLogger log(gopts, "MySubsystem", "mysubsystem");
 * log.info("starting run ", runNumber);
 * \endcode
 *
 * \subsection ex_levels_sec Example: verbosity-gated information
 * This example shows how to emit progressively more detailed messages depending on verbosity.
 *
 * \code
 * log.info(0, "run header...");
 * log.info(1, "loaded geometry variant ", variantName);
 * log.info(2, "per-channel thresholds: ", thresholdsSummary);
 * \endcode
 *
 * \subsection ex_debug_sec Example: developer-focused debug output
 * This example shows debug output that appears only when debug is enabled for the logger name.
 *
 * \code
 * log.debug(NORMAL, "constructed calibration map with ", nEntries, " entries");
 * \endcode
 *
 * \n\n
 * \author \n &copy; Maurizio Ungaro
 * \author e-mail: ungaro@jlab.org
 * \n\n\n
 */
