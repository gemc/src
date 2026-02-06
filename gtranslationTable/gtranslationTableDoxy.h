#ifndef  GTRANSLATIONTABLE_H_DOXY
#define  GTRANSLATIONTABLE_H_DOXY 1

/**
 * \mainpage Translation Table Module (gtranslationTable)
 *
 * \section intro_sec Introduction
 *
 * The Translation Table module provides a compact mapping between a vector-based identity
 * (a \c std::vector<int>) and an electronics configuration object (GElectronic).
 *
 * The primary class is GTranslationTable. It converts an identity vector into a stable,
 * hyphen-separated key string and stores the associated GElectronic instance in an internal map.
 *
 * \section ownership_sec Ownership and scope
 *
 * - **Owner/Maintainer**: Maurizio Ungaro
 * - **Scope**: utility infrastructure for associating detector/channel identities to electronics
 *   configurations and retrieving them efficiently during simulation and digitization workflows.
 *
 * \section api_overview_sec API overview
 *
 * The typical workflow is:
 * 1. Construct a GTranslationTable with the module options.
 * 2. Register one or more GElectronic configurations for specific identities.
 * 3. Retrieve the electronics configuration by providing the same identity vector.
 *
 * Relevant public methods:
 * - \ref GTranslationTable::addGElectronicWithIdentity "addGElectronicWithIdentity()"
 * - \ref GTranslationTable::getElectronics "getElectronics()"
 *
 * @section options_sec Available Options and their usage
 *
 * This module currently does not define or consume any module-specific option keys.
 *
 * Notes:
 * - The module is constructed with a \c std::shared_ptr<GOptions> primarily so it can participate
 *   in the standard logging configuration pathway (via \c TRANSLATIONTABLE_LOGGER).
 * - Global keys defined by \ref GOptions::GOptions "GOptions(argc,argv,...)" (e.g. `verbosity`, `debug`)
 *   may still affect the logger behavior associated with this module.
 *
 * \section verbosity_sec Module verbosity
 *
 * This module logs through the project logger infrastructure (GLogger) by way of GBase.
 * The logger name for this module is \c "gtranslationtable" (see TRANSLATIONTABLE_LOGGER).
 *
 * Typical meaning of verbosity levels:
 * - **Level 0**: essential user-facing information (high-level results).
 * - **Level 1**: important workflow milestones (e.g. registrations performed).
 * - **Level 2**: more detailed progress information (useful when diagnosing configuration issues).
 *
 * Debug output (e.g. \c log->debug(...)) prints diagnostic details such as the current
 * translation table content and key lookups, and is intended for development/troubleshooting.
 *
 * \section examples_sec Examples (Table of contents)
 *
 * \subsection example_tt_basic_sec examples/tt_example.cc
 * **Summary**: Demonstrates how to create a Translation Table, register two identities, retrieve
 * a configuration, and print the result via the module logger.
 *
 * \section notes_sec Notes and conventions
 *
 * - Identity vectors are converted to a hyphen-separated key (e.g. \c 1-2-3-4-5).
 * - Keys must be stable: the same identity vector must be provided to retrieve the same entry.
 * - When an entry is missing, the module logs an error with EC__TTNOTFOUNDINTT.
 *
 * \n\n
 * \author \n &copy; Maurizio Ungaro
 * \author e-mail: ungaro@jlab.org
 * \n\n\n
 */

#endif
