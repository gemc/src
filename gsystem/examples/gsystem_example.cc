/**
* \file gsystem_example.cc
 * \brief Minimal example: construct a world from command-line options.
 *
 * \ingroup gemc_gsystem_examples
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

/**
 * \brief Program entry point.
 *
 * \param argc Argument count from the command line.
 * \param argv Argument vector from the command line.
 * \return EXIT_SUCCESS on clean construction/destruction.
 *
 * \details
 * The world is allocated and then explicitly deleted to match existing ownership style.
 * In user code, prefer automatic storage or smart pointers where possible.
 */
#include "gworld.h"

// the geometry comes linked from one of the examples
int main(int argc, char* argv[]) {
	auto gopts = std::make_shared<GOptions>(argc, argv, gsystem::defineOptions());
	auto world = new GWorld(gopts);

	delete world;
	return EXIT_SUCCESS;
}
