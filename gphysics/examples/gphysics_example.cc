/**
* @file gphysics_example.cc
 * @anchor gphysics_example_main
 *
 * @ingroup gphysics_module
 *
 * @brief Minimal example showing how to define gphysics options and instantiate GPhysics.
 *
 * This example demonstrates the typical integration pattern:
 * - Create a GOptions object using gphysics::defineOptions().
 * - Instantiate GPhysics with the shared options.
 *
 * The example is intentionally minimal and does not attach the resulting physics list to a
 * Geant4 run manager. It is meant as a compilation and usage reference for the module API.
 *
 * Usage:
 * @code
 * ./gphysics_example -phys_list FTFP_BERT
 * ./gphysics_example -showPhysics
 * @endcode
 */

 // gphysics
#include "gphysics.h"
#include "gphysics_options.h"

// goptions
#include "goptions.h"

/**
 * @brief Program entry point.
 *
 * This function:
 * - Parses command line arguments into a GOptions instance configured with gphysics::defineOptions().
 * - Instantiates GPhysics, which will either:
 *   - Print available physics lists/constructors if \c showPhysics is enabled, or
 *   - Build the reference physics list selected by \c phys_list.
 *
 * @param argc Number of command line arguments.
 * @param argv Array of command line argument strings.
 * @return Process exit code (\c EXIT_SUCCESS on normal completion).
 */
int main(int argc, char* argv[]) {
	auto gopts =  std::make_shared<GOptions>(argc, argv, gphysics::defineOptions());

	[[maybe_unused]] auto gphysics = std::make_shared<GPhysics>(gopts);

	return EXIT_SUCCESS;
}
