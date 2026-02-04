/**
 * @mainpage GEMC Geant4 System
 *
 * @section intro_sec Introduction
 * The g4system module is the **geometry factory layer** that translates detector
 * descriptions stored in databases (ASCII / SQLite / GDML / CAD) into runtime Geant4
 * volumes (solid/logical/physical).
 *
 * The module is designed so that you can add new detector systems by implementing
 * a factory and registering it, without changing the core world-building logic.
 *
 * @section modules_sec Module overview
 * - @ref Factory   “Factory / Plugin Loader” – runtime creation of C++ classes
 * - @ref Geometry  “Geometry Builders”       – conversion from GEMC DB → Geant4
 *
 * @section verbosity_sec Verbosity and debug output
 * Most classes in this module use the common logging infrastructure (classes derived from the
 * logger-enabled base). The following behavior is typical:
 * - Verbosity level 0 prints essential progress and high-level configuration (major steps,
 *   critical warnings).
 * - Verbosity level 1 prints additional context useful during standard validation (counts,
 *   selected materials, basic dependency messages).
 * - Verbosity level 2 prints detailed traces useful while developing geometry/factories
 *   (per-volume build results, dependency resolution steps, material/component listings).
 *
 * Debug messages provide fine-grained diagnostics intended for development and troubleshooting,
 * such as pointer assignments, dependency checks, and factory dispatch details.
 *
 * @section usage_sec Quick-start
 * Example of building a world from options:
 * @code{.cpp}
 * auto gopts  = std::make_shared<GOptions>(argc, argv, g4system::defineOptions());
 * auto gworld = std::make_shared<GWorld>(gopts);
 * auto g4w    = std::make_shared<G4World>(gworld.get(), gopts);
 * @endcode
 *
 * @section examples_sec Examples
 * - \ref g4system_example_anchor "g4system_example.cc" : Minimal instantiation of the module, world creation,
 *   and basic diagnostic queries (volume count, sensitive detector list).
 *
 * \author &copy; Maurizio Ungaro
 * \author e-mail: ungaro@jlab.org
 */


/**
 * @defgroup G4System GEMC Geant4 System
 * @brief Geant4 geometry construction module within GEMC.
 *
 * This top-level group contains the g4system submodules responsible for translating
 * GEMC detector descriptions into Geant4 geometry objects.
 */


/**
 * @defgroup Factory Factory / Plugin Loader
 * @ingroup G4System
 * @brief Runtime creation of system builders and object factories.
 *
 * The classes in this group implement the plug-in mechanism that allows GEMC to
 * instantiate detector-specific builders at runtime based on the factory name stored
 * in the database configuration.
 *
 * Typical responsibilities:
 * - Register available factories with a manager.
 * - Create a concrete factory instance by name.
 * - Provide a common initialization context (options, logging, overlap checks, etc.).
 */


/**
 * @defgroup Geometry Geometry Builders
 * @ingroup G4System
 * @brief Builders that convert GEMC volume/material records into Geant4 geometry.
 *
 * This group contains the world builder and object factories responsible for translating
 * GEMC geometry records into:
 * - Geant4 solids
 * - Geant4 logical volumes (material + visual attributes)
 * - Geant4 physical volumes (placements and hierarchy)
 *
 * The core design supports out-of-order construction by performing dependency checks and
 * retrying volumes/materials until prerequisites (mother volume, copy-of source, boolean
 * operands, etc.) are available.
 */
