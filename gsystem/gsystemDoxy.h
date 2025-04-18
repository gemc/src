/**
 * \mainpage GEMC Detector System
 *
 * @section intro_sec Introduction
 *
 * This documentation describes the GSystem class, which is a fundamental component
 * for defining detector geometries within the simulation framework.
 * A GSystem represents a logical collection of volumes (GVolume) and associated
 * materials (GMaterial) that form a specific detector subsystem (e.g., calorimeter,
 * drift chamber).
 *
 * @section usage_sec Usage
 *
 * Typically, a GSystem is instantiated with a name, factory type, variation,
 * and run number. Volumes and materials are then added to it using methods like
 * addGVolume(), addGMaterial(), or addVolumeFromFile(). The system provides
 * accessors to retrieve information about its configuration and components.
 *
 * @section design_sec Design Notes
 *
 * - Uses modern C++ features like smart pointers (`std::unique_ptr`) for automatic memory management.
 * - Employs a logger (`GLogger`) for informative messages and error reporting.
 * - Aims for clear separation of concerns, with GSystem managing the collection
 *   and GVolume/GMaterial representing the individual components.
 * - Materials are considered specific to the system; they are not shared between GSystem instances
 *   at this level (global materials might be handled elsewhere, e.g., via a G4Material database).
 *
 * \n\n
 * \author \n &copy; Maurizio Ungaro
 * \author e-mail: ungaro@jlab.org
 * \n\n\n
 */