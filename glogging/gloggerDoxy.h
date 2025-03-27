/**
 *
 * \mainpage GLogger Module
 *
 * \section intro_sec Introduction
 * The GLogger module is responsible for handling structured logging in the simulation.
 * It supports various log levels (debug, info, warning, error, and critical) with configurable
 * verbosity and debug settings. Log messages are prepended with a header that includes a unique
 * counter to help trace message order in multi-threaded environments.
 *
 * \section details_sec Details
 * - Uses modern C++ features such as variadic templates, fold expressions, perfect forwarding, and
 *   [[nodiscard]] attributes.
 * - Provides a robust logging interface where debug messages can be suppressed if debug is off.
 * - The logger uses atomic counters to ensure thread safety.
 *
 * \section usage_sec Usage
 * Instantiate a GLogger with a pointer to a GOptions object and a string identifier. Then call
 * the member functions (debug, info, warning, error, critical) to log messages. The error function
 * terminates the process.
 *
 * \n\n
 * \author \n &copy; Maurizio Ungaro
 * \author e-mail: ungaro@jlab.org
 * \n\n\n
 */