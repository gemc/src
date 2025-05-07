/**
* @mainpage GEMC Geant4 System
 *
 * @section intro_sec Introduction
 * The GEMC **geometry factory layer** translates detector descriptions stored in
 * databases (ASCII / SQLite / GDML / CAD) into run‑time Geant4 volumes.
 * This separation lets you add new detector systems with zero changes to the core.
 *
 * @section modules_sec Module overview
 * - @ref Factory   “Factory / Plugin Loader” – runtime creation of C++ classes
 * - @ref Geometry  “Geometry Builders”      – conversion from GEMC DB → G4
 *
 * @section usage_sec Quick‑start
 * ```cpp
 * auto manager = std::make_unique<gemc::GManager>(log, "Detectors");
 * manager->RegisterObjectFactory<MyCalorimeter>("ecal");
 * auto* ecal = manager->CreateObject<gemc::G4ObjectsFactory>("ecal");
 * ```
 *
/// \n
/// \n\n
/// \author \n &copy; Maurizio Ungaro
/// \author e-mail: ungaro@jlab.org
/// \n\n\n
///
