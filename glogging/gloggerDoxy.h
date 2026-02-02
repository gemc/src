/**
 *
 * \mainpage GLogger Module
 *
 * \section intro_sec Introduction
 * The GLogger module provides structured, consistently formatted logging for the simulation runtime.
 * It is intended to be the single place where verbosity/debug policies and log formatting rules
 * are applied so that the rest of the codebase can emit messages without re-implementing
 * filtering, headers, or formatting.
 *
 * The logger supports multiple message categories:
 * - Debug messages (optionally filtered by a debug level)
 * - Informational messages (optionally filtered by a verbosity level)
 * - Warnings (always printed)
 * - Errors (printed and then the process terminates with an exit code)
 * - Critical messages (always printed, emphasized formatting)
 *
 * Each emitted message is prepended with a compact header that includes:
 * - The configured logger name (a logical subsystem identifier)
 * - A monotonically increasing counter (per logger instance)
 *
 * The counter is particularly useful in multi-threaded environments because it helps correlate
 * message order even when different threads interleave output.
 *
 * \section details_sec Implementation notes
 * - The implementation is intentionally lightweight and header-driven.
 * - Debug/info/warning/error/critical accept a variadic list of "streamable" arguments, which are
 *   concatenated using an `std::ostringstream`.
 * - A `std::atomic<int>` counter is used to produce the per-instance sequence number in a
 *   thread-safe manner.
 * - Debug logging supports special "constructor/destructor" styles to make lifetime tracing easier.
 *
 * \section usage_sec Usage
 * 1. Construct a \ref GLogger "GLogger" instance with:
 *    - A shared pointer to \ref GOptions "GOptions" (used to resolve verbosity/debug configuration)
 *    - A caller class name (informational; currently not used for filtering)
 *    - A logger name (used as a lookup key in \ref GOptions "GOptions")
 * 2. Call the relevant message methods:
 *    - \ref GLogger::debug "debug()"
 *    - \ref GLogger::info "info()"
 *    - \ref GLogger::warning "warning()"
 *    - \ref GLogger::error "error()"
 *    - \ref GLogger::critical "critical()"
 *
 * \note The \ref GLogger::error "error()" method is marked `[[noreturn]]` and terminates the process.
 *
 * \n\n
 * \author \n &copy; Maurizio Ungaro
 * \author e-mail: ungaro@jlab.org
 * \n\n\n
 */
