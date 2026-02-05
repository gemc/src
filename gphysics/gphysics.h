#pragma once

// gemc
#include "gbase.h"

// geant4
#include "G4VModularPhysicsList.hh"

/**
 * @class GPhysics
 * @ingroup gphysics_module
 *
 * @brief Builds and exposes a Geant4 modular physics list based on runtime options.
 *
 * GPhysics is a thin integration layer between GEMC options and the Geant4 extensible physics list
 * factory. It is not derived from \c G4VModularPhysicsList; instead, it creates a physics list instance
 * and returns a pointer to it via getPhysList().
 *
 * The physics list selection is driven by options defined in gphysics::defineOptions(), most notably:
 * - \c phys_list : the reference physics list string (with optional extensions).
 * - \c showPhysics : a switch that prints the available physics lists and constructors and exits early.
 *
 * Ownership note:
 * The returned \c G4VModularPhysicsList pointer is expected to be managed by the Geant4 application
 * lifecycle (typically by the run manager). This class does not delete the physics list.
 */
class GPhysics : public GBase<GPhysics> {
public:
	/**
	 * @brief Constructs the physics list builder and (unless requested otherwise) instantiates the physics list.
	 *
	 * The constructor reads the relevant options from the provided GOptions instance:
	 * - If the \c showPhysics switch is enabled, the constructor prints the available Geant4 lists/constructors
	 *   and returns without creating a physics list.
	 * - Otherwise, it requests the reference physics list specified by the \c phys_list option and registers
	 *   additional constructors required by the module defaults.
	 *
	 * @param gopts Shared options container used to retrieve \c phys_list and \c showPhysics.
	 */
	GPhysics(const std::shared_ptr<GOptions>& gopts);

	/**
	 * @brief Returns the instantiated Geant4 modular physics list.
	 *
	 * @return Pointer to the \c G4VModularPhysicsList created by this module, or \c nullptr if the list
	 *         could not be created or if \c showPhysics was requested.
	 */
	[[nodiscard]] G4VModularPhysicsList* getPhysList() const { return physList; }

private:
	// Logs the available physics lists and constructors.
	// This is a private helper; user-facing behavior is documented at class level.
	void printAvailable() const;

	/**
	 * @brief Pointer to the physics list created through the Geant4 factory.
	 *
	 * This pointer is handed to the consumer (typically the application run manager).
	 * This class does not own the object lifetime.
	 */
	G4VModularPhysicsList* physList;
};
