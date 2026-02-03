#pragma once

/**
 * \mainpage
 *
 * \section goptions_main_overview Overview
 *
 * \ref GOptions : provides a unified framework to define, parse, validate, and persist configuration
 * coming from **YAML files** and **command-line arguments**. It is designed to support:
 * - small executables (a handful of options),
 * - large frameworks/plugins (many modules contributing options),
 * - reproducible batch workflows (automatic YAML snapshot of resolved configuration).
 *
 * The subsystem is built around:
 * - \ref GSwitch : a presence-based boolean flag (e.g. `-gui`).
 * - \ref GOption : an option value container that can be scalar or structured.
 * - \ref GVariable : a schema entry used when defining scalar options or structured option keys.
 *
 * \tableofcontents
 *
 * \section goptions_main_concepts Concepts
 *
 * \subsection goptions_main_switches Switches
 * A \ref GSwitch : is a lightweight boolean flag:
 * - default is **off**,
 * - specifying `-<name>` on the command line turns it **on**.
 *
 * Example:
 * \code{.sh}
 * myprog -gui
 * \endcode
 *
 * \subsection goptions_main_scalar Scalar options
 * A scalar \ref GOption : stores exactly one value as a YAML scalar. Values can be supplied via:
 * - YAML:
 * \code{.yaml}
 * runno: 12
 * \endcode
 * - Command line:
 * \code{.sh}
 * myprog -runno=12
 * \endcode
 *
 * \subsection goptions_main_structured Structured options
 * A structured \ref GOption : stores either:
 * - a YAML map, or
 * - a YAML sequence (commonly a sequence of maps).
 *
 * A typical pattern is a **sequence of maps** where each entry represents one repeated item:
 * \code{.yaml}
 * gparticle:
 *  - name: e-
 *    p: 1500
 *    theta: 23.0
 *    multiplicity: 4
 * \endcode
 *
 * Equivalent command-line option (quotes are usually required so your shell does not split the string):
 * \code{.sh}
 * myprog -gparticle="[{name: e-, p: 1500, theta: 23.0, multiplicity: 4}]"
 * \endcode
 *
 * \subsection goptions_main_cumulative Cumulative structured options and mandatory keys
 * Structured options can be declared **cumulative** when at least one schema key uses
 * \ref goptions::NODFLT : as its default value. That tells \ref GOption : that:
 * - the option expects a **sequence of maps** (multiple entries),
 * - those keys are **mandatory** and must appear in every entry.
 *
 * Missing non-mandatory keys are back-filled from schema defaults so every entry becomes complete.
 *
 * \subsection goptions_main_dot Dot-notation updates for structured sub-options
 * Some structured options are updated using dot-notation:
 * \code{.sh}
 * myprog -verbosity.general=1
 * myprog -debug.general=true
 * \endcode
 *
 * This updates a single subkey within a structured option via
 * \ref GOption::set_sub_option_value "set_sub_option_value()" .
 *
 * \section goptions_main_verbosity Verbosity and debug behavior
 *
 * This module defines two conventional structured options commonly used across the project:
 * - `verbosity` (integer levels)
 * - `debug` (boolean or integer)
 *
 * Typical semantics used by classes that consume these settings:
 * - Level **0**: minimal output ("shush") — only essential messages.
 * - Level **1**: detailed informational output — key configuration and progress.
 * - Level **2**: extra detailed output — expanded per-step or per-event diagnostics.
 * - `debug=true` (or debug > 0): developer-focused diagnostics — internal state, parsing details,
 *   and other troubleshooting information beyond normal verbosity.
 *
 * \section goptions_main_examples Examples
 *
 * The following example program is provided with this module:
 * - **examples/define_options.cc** : Defines switches and options (scalar + structured), constructs a parsing
 *   \ref GOptions : instance, and reads back resolved values with typed getters.
 *
 * \section goptions_main_extensibility Extensibility via merging option definitions
 *
 * Frameworks or plugins can define their own options and switches, then merge them into the executable’s
 * definition set using \ref operator+= "operator+=()" (which internally calls
 * \ref GOptions::addGOptions "addGOptions()").
 *
 * Example:
 * \code{.cpp}
 * goptions += eventDispenser::defineOptions();
 * goptions += g4display::defineOptions();
 * goptions += g4system::defineOptions();
 * \endcode
 *
 * \section goptions_main_yaml YAML parsing and validation
 *
 * YAML parsing is implemented using the \c yaml-cpp library. When a YAML file fails to parse, the option manager
 * exits with a dedicated exit code so batch workflows can detect the failure mode.
 *
 * \section goptions_main_ownership Ownership and maintenance
 *
 * The goptions module is maintained as part of GEMC and is intended to be:
 * - stable at the API level for consumers of \ref GOptions : ,
 * - strict and explicit about error reporting (exit codes) for batch reproducibility,
 * - documentation-first: headers contain authoritative API docs; implementation files avoid duplication.
 *
 * \author
 * \n &copy; Maurizio Ungaro
 * \n e-mail: ungaro@jlab.org
 */
