/**
 * @defgroup dbselect_module dbselect module
 *
 * @brief Qt-based selection view for SQLite-backed geometry “experiment/system” configurations.
 *
 * This module provides a small GUI/CLI-oriented utility layer to inspect and select
 * geometry configurations stored in an SQLite database (via the \c geometry table),
 * and to translate user selections into a list of GSystem objects that can be passed
 * to detector construction for geometry reload.
 */

/**
 * \mainpage dbselect
 *
 * @section dbselect_intro Introduction
 *
 * The dbselect module provides a Qt widget (\c DBSelectView) that presents experiments and
 * systems retrieved from an SQLite database. Users can enable/disable systems and choose
 * the corresponding variation and run, then trigger a geometry reload through a supplied
 * \c GDetectorConstruction instance.
 *
 * The typical flow is:
 * - Parse options into a GOptions instance using dbselect::defineOptions().
 * - Create a \c GDetectorConstruction.
 * - Create a \c DBSelectView and let the user modify selections.
 * - Collect the updated SystemList from \ref DBSelectView::get_gsystems "get_gsystems()"
 *   and invoke detector construction reload.
 *
 * @section dbselect_ownership Ownership and lifecycle
 *
 * Ownership and responsibilities are intentionally separated:
 * - \c DBSelectView owns its UI elements (Qt parent/child ownership) and manages the
 *   SQLite handle (\c sqlite3*) opened read-only.
 * - \c DBSelectView does not own the provided \c GDetectorConstruction pointer; it is
 *   treated as an external service used to reload geometry.
 * - The selection state is stored in the Qt model (\c QStandardItemModel) and mirrored
 *   into a SystemList only when requested.
 *
 * Lifecycle notes:
 * - The database is opened during \c DBSelectView construction and closed in the destructor.
 * - Changes in the model mark the view as “modified” and enable the Reload button.
 * - Calling \ref DBSelectView::reload_geometry "reload_geometry()" rebuilds the SystemList,
 *   invokes geometry reload on the detector construction, then clears the modified flag.
 *
 * @section dbselect_arch Architecture
 *
 * @subsection dbselect_arch_overview Design notes
 *
 * Internally the view is a thin integration of three concerns:
 * - **Data source:** an SQLite database containing a \c geometry table with at least the
 *   columns used by the queries (experiment/system/variation/run).
 * - **Selection model:** a \c QStandardItemModel with four columns:
 *   - column 0: experiment or system name (checkable)
 *   - column 1: entry count (computed from the database)
 *   - column 2: variation (edited via a drop-down delegate)
 *   - column 3: run (edited via a drop-down delegate)
 * - **Application bridge:** translating checked systems into a SystemList of \c GSystem
 *   objects via \ref DBSelectView::get_gsystems "get_gsystems()".
 *
 * UI behavior:
 * - Selecting an experiment checks only one experiment at a time.
 * - Enabling a system shows an availability indicator (green/red icon) driven by the
 *   computed count of matching geometry entries.
 * - Changing variation/run recomputes counts and availability.
 *
 * @section options_sec Available Options and their usage
 *
 * This module reads the following option keys from the runtime option provider:
 *
 * - `sql`
 *   - Type: string
 *   - Meaning: path (or identifier) of the SQLite database file to open (read-only)
 *   - Behavior:
 *     - the database is opened during \c DBSelectView construction
 *     - the value is also used as the default geometry source when building a \c SystemList
 *   - Note: this key is commonly provided by the aggregated gsystem/g4system option sets.
 *
 * - `experiment`
 *   - Type: string
 *   - Meaning: default experiment name to preselect in the view
 *   - Behavior:
 *     - when present, the view attempts to select that experiment on startup
 *     - if the experiment is not found, the view falls back to the first available experiment
 *
 * - `gui`
 *   - Type: boolean (switch)
 *   - Meaning: enable GUI execution path for examples / host applications
 *   - Behavior:
 *     - when \c true, example programs typically start a Qt event loop and show the widget
 *     - when \c false, examples may run in CLI mode (no widget created)
 *   - Note: this switch is defined by \ref GOptions::GOptions "GOptions(argc,argv,...)" and is globally available.
 *
 * This module’s option schema is composed by \c dbselect::defineOptions(), which aggregates:
 * - \c gdetector::defineOptions()
 *
 * Additional keys contributed by \c gdetector::defineOptions() may affect detector construction
 * and geometry reload behavior. Refer to the gdetector documentation for the full list.
 *
 * @section dbselect_verbosity Module verbosity
 *
 * The module uses the logger name \c "dbselect".
 *
 * Typical conventions used in this module:
 * - Level 0: high-level lifecycle messages (startup, mode selection, geometry reload trigger).
 * - Level 1: environment and configuration messages (database opened and resolved location).
 * - Level 2: detailed selection and rebuild information (systems being added and reloaded).
 * - Debug: reserved for verbose troubleshooting (not used directly in the provided sources).
 *
 * @section dbselect_examples Examples
 *
 * @subsection dbselect_examples_test_dbselect test_dbselect
 *
 * Summary:
 * The \ref test_dbselect "test_dbselect" example demonstrates how to instantiate a
 * \c GOptions configuration for dbselect, optionally start a Qt GUI, and trigger a
 * geometry reload from the view.
 *
 * Example snippet:
 * \code
 * auto gopts = std::make_shared<GOptions>(argc, argv, dbselect::defineOptions());
 * auto gdetector = new GDetectorConstruction(gopts);
 *
 * if (gopts->getSwitch("gui")) {
 *     auto dbselect = new DBSelectView(gopts, gdetector, window);
 *     window->setCentralWidget(dbselect);
 *     window->show();
 * }
 * \endcode
 */
