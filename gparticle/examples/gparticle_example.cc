/**
 * @file gparticle_example.cc
 * @brief Example program demonstrating option parsing for the gparticle module.
 *
 * This example shows how to:
 * - Build the gparticle option schema using gparticle::defineOptions()
 * - Parse command-line options into a GOptions instance
 * - Create a module logger
 * - Build a vector of \ref Gparticle objects using gparticle::getGParticles()
 *
 * @anchor gparticle_example_main
 *
 * Usage example (shell):
 * @code
 * ./gparticle_example -gparticle="[{name: e-, p: 5000, punit: MeV}]"
 * @endcode
 *
 * Notes:
 * - This program initializes a \c G4RunManager and sets a simple physics list.
 * - The example does not run an event loop; it focuses on configuration parsing
 *   and object construction.
 */

// gparticle
#include "gparticle_options.h"

// geant4
#include "G4RunManagerFactory.hh"
#include "QBBC.hh"


/**
 * @brief Program entry point.
 *
 * The program performs the following steps:
 * 1. Creates a GOptions instance that includes the gparticle option definitions.
 * 2. Creates a logger for the module (using the GPARTICLE_LOGGER channel).
 * 3. Creates a \c G4RunManager using the factory API.
 * 4. Installs a minimal physics list (\c QBBC).
 * 5. Parses the configured particles by calling gparticle::getGParticles().
 * 6. Cleans up the run manager and exits.
 *
 * @param argc Number of command-line arguments.
 * @param argv Command-line arguments.
 * @return Process exit code.
 */
int main(int argc, char* argv[]) {
	// Build options with the gparticle schema so -gparticle can be parsed.
	auto gopts = std::make_shared<GOptions>(argc, argv, gparticle::defineOptions());

	// Create a module logger; verbosity controls how much configuration is printed.
	auto log = std::make_shared<GLogger>(gopts, FUNCTION_NAME, GPARTICLE_LOGGER);

	// Create a Geant4 run manager and install a basic physics list.
	auto runManager = G4RunManagerFactory::CreateRunManager(G4RunManagerType::Default);

	auto physicsList = new QBBC;
	runManager->SetUserInitialization(physicsList);

	// Build particles from the structured -gparticle option node.
	auto particles = gparticle::getGParticles(gopts, log);

	// This example does not execute events; it demonstrates construction and parsing only.
	delete runManager;

	return EXIT_SUCCESS;
}
