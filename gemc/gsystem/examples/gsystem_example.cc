/**
* \file gsystem_example.cc
 * \ingroup gemc_gsystem_examples
 *
 * \anchor gsystem_example_anchor
 *
 * \brief Minimal example : construct a world from command-line options.
 *
 * \details
 * This example demonstrates the smallest executable that:
 * - creates a GOptions instance using gsystem module option definitions;
 * - constructs a GWorld, which loads systems, volumes, and (optional) modifiers.
 *
 * Typical usage:
 * \code
 * ./gsystem_example -gsystem="[{name: b1, factory: sqlite, variation: default}]" -sql=gemc.db
 * \endcode
 *
 * The example intentionally performs no further actions beyond construction/destruction;
 * it is meant as a smoke test for configuration and factory wiring.
 *
 * \author \n &copy; Maurizio Ungaro
 */

#include "gworld.h"

/**
 * \brief Program entry point.
 *
 * \param argc Argument count from the command line.
 * \param argv Argument vector from the command line.
 * \return EXIT_SUCCESS on clean construction/destruction.
 *
 * \details
 * Execution flow:
 * - Build a shared GOptions instance using \c gsystem::defineOptions().
 * - Construct a GWorld, which triggers full loading:
 *   - system discovery via \c gsystem::getSystems();
 *   - factory instantiation;
 *   - materials loading;
 *   - geometry loading;
 *   - modifier loading and application;
 *   - final Geant4 name assignment bookkeeping.
 *
 * This program uses explicit \c new / \c delete to match existing ownership style in examples.
 * In user applications, prefer automatic storage duration or smart pointers where appropriate.
 */
int main(int argc, char* argv[]) {
	auto gopts = std::make_shared<GOptions>(argc, argv, gsystem::defineOptions());
	auto world = new GWorld(gopts);

	delete world;
	return EXIT_SUCCESS;
}
