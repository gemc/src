/**
 * @defgroup gtree_module GTree module
 * @brief Qt-based geometry tree browser for interactive visualization control.
 *
 * @details
 * This module provides a Qt widget that presents the detector geometry as a
 * hierarchical tree. It enables users to toggle visibility and adjust visual
 * attributes (color, opacity, representation) for individual volumes.
 *
 * The group is intentionally named with a module-specific prefix to avoid
 * collisions across tag-file modules.
 */

/**
 * \mainpage GTree
 *
 * @section intro_sec Introduction
 *
 * GTree is a Qt widget that exposes a hierarchical view of the Geant4 geometry.
 * It is designed for interactive inspection and quick style adjustments:
 * - Toggle volume visibility (and propagate to direct children)
 * - Change volume color
 * - Adjust opacity (alpha channel)
 * - Switch representation (wireframe / solid / cloud)
 *
 * The widget is built from a map of geometry volumes (full names including a
 * system prefix), organized by system and then by mother-daughter relations.
 *
 *
 * @section ownership_sec Ownership and lifecycle
 *
 * - **Ownership model**:
 *   - The widget owns its Qt UI elements via Qt parent-child ownership.
 *   - The geometry "model" stored in memory (`G4Ttree_item` objects) is owned by
 *     this module (`std::unique_ptr` inside internal maps).
 *   - The Geant4 geometry itself is not owned by this widget. The widget sends
 *     visualization commands to the Geant4 UI manager to affect rendering.
 *
 * - **Lifecycle**:
 *   - Constructed with a pre-built geometry volume map.
 *   - Builds an internal representation (`g4_systems_tree`) once at construction.
 *   - UI interactions send visualization commands immediately.
 *
 *
 * @section architecture_sec Architecture
 *
 * **Design notes**
 * - **Model**: `G4Ttree_item` stores per-volume attributes needed by the UI
 *   (mother name, material, mass, volume, density, color, opacity, visibility).
 * - **View**: `QTreeWidget` shows a three-column tree:
 *   - Column 0: visibility checkbox
 *   - Column 1: color button
 *   - Column 2: volume display name
 * - **Controller**: slots react to user actions and send commands:
 *   - Visibility toggles emit `/vis/geometry/set/visibility ...`
 *   - Color / opacity updates emit `/vis/geometry/set/colour ...`
 *   - Representation toggles emit `/vis/geometry/set/force* ...`
 *
 * This module intentionally uses Geant4 UI commands to avoid direct coupling
 * to Geant4 visualization internals.
 *
 *
 * @section options_sec Available Options and their usage
 *
 * This module currently does not define additional module-specific option keys.
 *
 * The module’s option schema is composed by \c gtree::defineOptions(), which aggregates:
 * - \c dbselect::defineOptions()
 *
 * As a result, the following commonly used keys apply when running the example programs:
 *
 * - `sql`
 *   - Type: string
 *   - Meaning: sqlite database file used to load geometry/system configuration
 *   - Behavior:
 *     - passed through the aggregated dbselect/gsystem option sets
 *
 * - `experiment`
 *   - Type: string
 *   - Meaning: default experiment selection used by database-backed loaders and selectors
 *   - Behavior:
 *     - may be used to choose which experiment subtree is presented/selected upstream
 *
 * - `gui`
 *   - Type: boolean (switch)
 *   - Meaning: enable GUI execution path for examples
 *   - Behavior:
 *     - when \c true, example programs typically start a Qt event loop and show the widget
 *   - Note: this switch is defined by \ref GOptions::GOptions "GOptions(argc,argv,...)" and is globally available.
 *
 * Refer to the dbselect documentation for the full, authoritative list of aggregated keys.
 *
 *
 * @section verbosity_sec Module verbosity
 *
 * Logging is performed through the module logger named `gtree`.
 *
 * Typical meaning of levels:
 * - **Level 0**: high-level lifecycle messages (creation, major actions).
 * - **Level 1**: user-facing operational messages (UI mode, major state changes).
 * - **Level 2**: detailed diagnostics (volume inventory, computed properties).
 * - **Debug**: fine-grained internal traces (slot-level state, UI command strings).
 *
 *
 * @section examples_sec Examples
 *
 * - @ref gtree_example "gtree_example" :
 *   A minimal application that builds a detector and launches the GTree widget.
 *
 * Example snippet (from @ref gtree_example "gtree_example"):
 * @code
 * auto gopts = std::make_shared<GOptions>(argc, argv, gtree::defineOptions());
 * auto gdetector = new GDetectorConstruction(gopts);
 * runManager->SetUserInitialization(gdetector);
 * runManager->Initialize();
 * auto g4vmap = gdetector->get_g4volumes_map();
 * auto widget = new GTree(gopts, g4vmap, window);
 * @endcode
 *
 * \n\n
 * \author \n &copy; Maurizio Ungaro
 * \author e-mail: ungaro@jlab.org
 * \n\n\n
 */
