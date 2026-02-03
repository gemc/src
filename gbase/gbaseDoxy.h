#pragma once

/**
 * @file gbaseDoxy.h
 * @brief Doxygen entry point for the Base module.
 *
 * This header is documentation-only and is meant to be used as the primary landing
 * page for the module in generated Doxygen output.
 *
 * Ownership:
 * - Maintained by the GEMC core team.
 * - This file should remain free of implementation code (documentation only).
 */

/**
 * @mainpage Base Module
 *
 * @section base_overview Overview
 * The Base module provides small, reusable building blocks that higher-level
 * components can inherit or depend on. Its primary purpose is to standardize
 * common infrastructure concerns (most notably logging) without imposing
 * additional runtime or architectural constraints.
 *
 * The module is intentionally lightweight:
 * - It relies on compile-time patterns where possible (e.g., CRTP).
 * - It keeps ownership semantics explicit via \c std::shared_ptr.
 * - It avoids coupling to external frameworks in its public API documentation.
 *
 * @section base_key_types Key types
 * The module currently centers around:
 * - \ref GBase : A CRTP base class that equips derived objects with a ready-to-use logger.
 * - \ref GLogger : The logging facility used by \ref GBase .
 * - GOptions : The options/configuration mechanism used to initialize \ref GLogger .
 *
 * @section base_gbase GBase responsibilities
 * \ref GBase is designed to solve a recurring pattern in component implementations:
 * ensuring that each component can emit consistent log messages with minimal boilerplate.
 *
 * It provides two common construction modes:
 * - Dedicated logger per object:
 *   - Construct \ref GBase with a \c std::shared_ptr<GOptions>.
 *   - \ref GBase creates a \ref GLogger configured from the provided options.
 * - Shared logger across objects:
 *   - Construct \ref GBase with an existing \c std::shared_ptr<GLogger>.
 *   - Multiple objects can reuse the same logger to reduce initialization cost and
 *     keep log routing consistent.
 *
 * It also standardizes lifecycle logging:
 * - On construction, \ref GBase emits a constructor message through its logger.
 * - On destruction, \ref GBase emits a destructor message when a logger is available.
 *
 * @section base_verbosity Verbosity and debugging
 * Many components in GEMC rely on the logging conventions exposed by \ref GLogger and
 * used by \ref GBase. In typical usage:
 * - \c info(0, ...) is for essential, user-facing messages (minimal, high signal).
 * - \c info(1, ...) is for additional operational context (useful progress/detail).
 * - \c info(2, ...) is for very verbose informational output (diagnostics, repeated status).
 * - \c debug(...) is for developer-focused traces (function-level flow, state snapshots,
 *   and messages intended primarily for debugging).
 *
 * Exact routing/enablement is controlled by the logger configuration (via options).
 *
 * @section base_usage Typical usage
 * The common usage pattern is CRTP inheritance:
 * \code
 * class MyComponent : public GBase<MyComponent> {
 * public:
 *   explicit MyComponent(const std::shared_ptr<GOptions>& gopt)
 *     : GBase(gopt, "my_logger_channel") {}
 *
 *   void run() {
 *     log->info(0, "running");
 *   }
 * };
 * \endcode
 *
 * Notes:
 * - The derived class accesses the logger via the protected member \c log.
 * - The derived class does not need to implement any logging bootstrapping logic.
 * - Passing an empty logger channel name typically selects default behavior in the
 *   configured logging backend.
 *
 * @section base_examples Examples
 * The examples below are included in the repository as reference implementations and
 * are intentionally small. They are meant to be read alongside the header documentation.
 *
 * @subsection base_example_test_gbase Example: \c examples/test_gbase.cc
 * Demonstrates:
 * - Constructing a derived type that creates its own logger from options.
 * - Constructing a derived type that reuses a shared logger instance.
 * - Emitting \c info, \c warning, and \c debug messages from derived code.
 *
 * @section base_non_goals Non-goals
 * The Base module does not attempt to:
 * - Provide a full application framework.
 * - Hide ownership or lifetime rules behind implicit global state.
 * - Wrap or re-document third-party libraries in its public documentation.
 *
 * @section base_files Files in this module
 * - \c gbase.h : Defines \ref GBase and the type-name demangling helper used for readable diagnostics.
 * - \c gbaseDoxy.h : This documentation entry point and module landing page.
 * - \c examples/test_gbase.cc : A small usage example demonstrating both logger construction modes.
 *
 * @section base_navigation Where to look next
 * - Start with \ref GBase to understand the base class contract and expected inheritance pattern.
 * - Review \ref GLogger for logging configuration and routing behavior.
 * - Consult the examples to see typical construction patterns.
 */
