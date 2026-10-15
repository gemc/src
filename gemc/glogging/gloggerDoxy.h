/**
 *
 * \mainpage GLogger Module
 *
 * \tableofcontents
 *
 * \image html glogging-flow.svg "Structured log filtering and emission" width=900px
 *
 * \section glogging_intro Introduction
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
 * \section glogging_categories Message categories
 * The logger supports multiple message categories:
 * - **Debug** messages (filtered by a debug level)
 * - **Informational** messages (filtered by a verbosity level)
 * - **Warnings** (always printed)
 * - **Errors** (printed and then the process terminates with an exit code)
 * - **Critical** messages (always printed with emphasized formatting)
 *
 * \section glogging_verbosity Verbosity and debug levels
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
 * @section glogging_options Available Options and their usage
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
 *     - values are commonly `false`/`true` (mapped to 0/1) or an integer level, depending on consumer
 *       expectations
 *
 * Note: \ref GOptions::GOptions "GOptions(argc,argv,...)" defines these keys as global conventions so any
 * module can participate in consistent logging without re-defining these options.
 *
 * \section glogging_header Message header format
 * Each emitted message is prepended with a compact header that includes:
 * - The configured logger name (a logical subsystem identifier).
 * - A monotonically increasing counter (per logger instance).
 *
 * The counter is particularly useful in multi-threaded environments because it helps correlate
 * message order even when different threads interleave output.
 *
 * \section glogging_details Implementation notes
 * - Message formatting lives in the header; language detection and cached YAML loading live in the
 *   catalog module.
 * - Debug/info/warning/error/critical accept a complete message template followed by streamable
 *   parameter values. Parameters are substituted once without interpreting their text as templates.
 * - A \c std::atomic<int> counter is used to produce the per-instance sequence number in a
 *   thread-safe manner.
 * - Debug logging supports special constructor/destructor styles to make lifetime tracing easier.
 *
 * \section glogging_usage Usage
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
 * \section glogging_templates Complete message templates
 * Upcoming in the next release: `info()`, `debug()`, `warning()`, `error()`, and `critical()` accept
 * a complete message and separate parameter values. The transitional `*Template()` names forward to
 * the same methods; the former concatenating calls must be migrated.
 *
 * \code{.cpp}
 * log.info(0, "Using %1 threads out of %2 available cores.", useThreads, ncores);
 * log.info("Starting run %1.", runNumber); // Defaults to info level 0.
 * log.warning("File %1 was not found; using %2.", filename, fallback);
 * log.debug(NORMAL, "Loaded %1 entries from %2.", count, filename);
 * log.error(exitCode, "Cannot read file %1.", filename);
 * \endcode
 *
 * Placeholders `%1` through `%99` refer to the one-based argument positions and may be reordered or
 * repeated. Every supplied argument must be referenced. Each value is streamed once, and inserted
 * values are never interpreted as additional placeholders. `%%` emits a literal percent sign; other
 * percent signs are literal. For example, `"%1%% complete"` with the value `50` produces `"50% complete"`.
 *
 * An invalid, missing, or unused parameter throws `std::invalid_argument` before any message is emitted.
 * Info/debug filtering happens before formatting, so suppressed messages neither stream their values
 * nor validate their placeholders. Argument expressions are still evaluated by C++ before the call.
 * Each parameter uses its own stream; stream manipulators do not carry over between parameters.
 * Output categories, colors, counters, verbosity validation, and fatal exit codes follow the existing API.
 * Catalog lookup happens before substitution; messages without translations use their English templates.
 *
 * \subsection glogging_migration Migrating concatenating calls
 * Keep the complete sentence in the template and replace each value with its numbered placeholder:
 * \code{.cpp}
 * // Before: log.info(0, "Starting run ", runNumber, ".");
 * log.info(0, "Starting run %1.", runNumber);
 * // Treat dynamic or already-formatted diagnostics as parameter data:
 * log.warning("%1", diagnosticText);
 * \endcode
 * Logging a dynamic string as a value preserves percent signs in filenames or external diagnostics.
 * Rebuild external plugins against matching GEMC headers after updating their logging calls.
 *
 * \section glogging_catalogs Generated message catalogs
 * Upcoming in the next release: `ci/extract_messages.py` extracts literal logging templates selected by
 * `translations/catalog.yaml` into `translations/en.yaml`. English C++ remains the source of truth.
 * Identical templates share one entry, with source locations recorded as comments. Documentation examples,
 * tests, and messages containing only dynamic data are excluded. A separate manifest and English catalog
 * belong to CLAS12; the tool accepts `--root` to select that owning repository.
 *
 * \code{.sh}
 * build/subprojects/pygemc/python_env/bin/python ci/extract_messages.py extract
 * build/subprojects/pygemc/python_env/bin/python ci/extract_messages.py extract --check
 * build/subprojects/pygemc/python_env/bin/python ci/extract_messages.py validate translations/en.yaml
 * build/subprojects/pygemc/python_env/bin/python ci/extract_messages.py validate-tree
 * \endcode
 *
 * `extract --check` fails when the generated catalog is stale. Catalog validation rejects duplicate keys,
 * empty translations, and changed parameter positions; reordering and repetition are allowed. Translated
 * catalogs may be partial, using English fallback. The shared Claude/Codex workflow generates language
 * catalogs through the existing weekday cron job, with validated, resumable batches and source-commit
 * tracking. See `translations/README.md` for the format, workflow commands, and scanner limits.
 * CI validates all catalogs without agents, rejecting altered placeholders, percent escapes, and required
 * whitespace. Missing messages report coverage and retain English fallback; agents review language accuracy.
 *
 * \section glogging_language Runtime language and lookup
 * Upcoming in the next release: `language: auto` selects the language without requiring a user option.
 * An explicit `-language=<code>` (or YAML scalar) overrides `GEMC_LANGUAGE`; automatic selection then
 * checks `LC_ALL`, `LC_MESSAGES`, and `LANG`, followed by native macOS/Windows settings or English.
 * `C` and `POSIX` locales select English. `it_IT.UTF-8` normalizes to `it-it`, with `it.yaml` used for
 * messages absent from `it-it.yaml`. `-language=en` forces English. Numeric formatting is unchanged.
 *
 * Meson installs catalogs in `<prefix>/<datadir>/gemc/translations/gemc`; CLAS12 owns a separate
 * `clas12` directory there. `GEMC_TRANSLATION_PATH` supplies optional colon-separated search directories.
 * Existing plugin search paths also expose neighboring catalog installations; recognized build trees
 * find the owning source catalogs. Regional entries precede base-language entries, then the first message
 * in search-path order wins. Missing or invalid catalogs retain English fallback.
 *
 * Matching logger configurations share an immutable \ref GMessageCatalog "GMessageCatalog" cache.
 * Invalid files produce one diagnostic per cached configuration; they cannot change fatal exit codes.
 * Message lookup performs no filesystem access, and suppressed messages do not perform lookup or substitution.
 * Rebuild plugins against matching headers and the `glogging` library. Restart to reload changed catalogs.
 * \ref GLogger::get_language "get_language()" reports the normalized requested language tag.
 *
 * \section glogging_examples Examples
 *
 * \subsection glogging_example_basic Example: basic logger construction and baseline info
 * This example shows how a subsystem creates a logger with a configured subsystem name and emits
 * always-on informational output.
 *
 * \code
 * // Suppose gopts is a std::shared_ptr<GOptions> configured elsewhere.
 * GLogger log(gopts, "MySubsystem", "mysubsystem");
 * log.info("starting run %1", runNumber);
 * \endcode
 *
 * \subsection glogging_example_levels Example: verbosity-gated information
 * This example shows how to emit progressively more detailed messages depending on verbosity.
 *
 * \code
 * log.info(0, "run header...");
 * log.info(1, "loaded geometry variant %1", variantName);
 * log.info(2, "per-channel thresholds: %1", thresholdsSummary);
 * \endcode
 *
 * \subsection glogging_example_debug Example: developer-focused debug output
 * This example shows debug output that appears only when debug is enabled for the logger name.
 *
 * \code
 * log.debug(NORMAL, "constructed calibration map with %1 entries", nEntries);
 * \endcode
 *
 * \n\n
 * \author \n &copy; Maurizio Ungaro
 * \author e-mail: ungaro@jlab.org
 * \n\n\n
 */
