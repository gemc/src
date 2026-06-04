/**
 * \defgroup Factory Factory
 * \brief Runtime factory registry and dynamic plugin loading utilities.
 *
 * This group contains the gfactory module public API:
 * - Static registration of concrete types by name and instantiation via a type-erased factory.
 * - Dynamic loading of plugin libraries and object creation via exported C symbols.
 *
 * The primary entry point is the GManager class.
 */

/**
 * \mainpage gfactory module
 *
 * \section overview Overview
 * The **gfactory** module provides a lightweight registry and loader for plugin-style objects.
 * It supports two complementary workflows:
 * - **Static factory registration**: register a concrete C++ type under a string key and instantiate it later.
 * - **Dynamic factory loading**: load a shared object at runtime and instantiate objects via a known C symbol.
 *
 * The primary entry point is the GManager class.
 *
 * @section options_sec Available Options and their usage
 *
 * This module currently does not define or consume any module-specific option keys.
 *
 * Notes:
 * - Many components using this module log through \c PLUGIN_LOGGER and therefore can be influenced
 *   by global logging controls (e.g. `verbosity`, `debug`) defined by \ref GOptions::GOptions "GOptions(argc,argv,...)".
 * - Plugin search paths and loading policies are typically implemented at the consumer level and documented
 *   by those consuming modules (e.g. gfields, g4system factories).
 *
 * \section conventions Conventions and expectations
 * - **Naming**: dynamic plugin files are expected to follow the convention `<name>.gplugin`.
 * - **Dynamic instantiation**:
 *   - A product base type (e.g., `Car`) typically provides a static method `instantiate(...)`
 *     that uses \c dlsym to locate an extern "C" factory function (e.g., `CarFactory`).
 *   - Derived implementations live in separate libraries and provide that extern "C" symbol.
 * - **Ownership**:
 *   - Static creation returns a raw pointer; the caller owns it and must delete it.
 *   - Dynamic creation returns `std::shared_ptr<T>` and keeps the library loaded for the lifetime of the object.
 *
 * \section examples Examples (see `examples/`)
 * The module ships with small examples intended as reference implementations:
 * - **static_and_dynamic_example.cc**:
 *   Demonstrates both static registration (for `Shape`) and dynamic loading (for `Car`) in a single program.
 * - **ShapeFactory.h / ShapeFactory.cc**:
 *   Implements two static factory products (`Triangle`, `Box`) derived from `Shape`.
 * - **TeslaFactory.* and FordFactory.* (dynamic)**:
 *   Implements two dynamically-loaded `Car` products and the exported `CarFactory` symbol.
 *
 * \section verbosity Verbosity and logging
 * Many classes in this ecosystem derive (directly or indirectly) from `glogger` infrastructure via `GBase`.
 * In practice, the module emits:
 * - **info level 0**: high-level lifecycle messages (e.g., library loaded successfully).
 * - **info level 1**: additional progress details useful for normal development runs.
 * - **info level 2**: more verbose informational traces (e.g., repeated operations in loops).
 * - **debug**: diagnostic details such as attempted search paths, symbol resolution, and cleanup ordering.
 *
 * Exact formatting and filtering depend on the logger configuration carried by `GOptions`.
 *
 * \section ownership Ownership and maintenance
 * Maintainer: Maurizio Ungaro (Jefferson Lab).
 *
 * \section contact Contact
 * e-mail: ungaro@jlab.org
 */
