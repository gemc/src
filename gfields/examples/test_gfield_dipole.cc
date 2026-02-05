/**
 * @defgroup gfield_examples GField examples
 * @ingroup gfield_module
 *
 * @brief Example programs demonstrating how to configure and use the GField module.
 *
 * Each example provides a minimal, focused workflow that can be referenced from the module mainpage.
 */

/**
 * @file test_gfield_dipole.cc
 * @ingroup gfield_examples
 * @brief Example program that loads a dipole field and evaluates it at random points.
 *
 * @anchor example_test_gfield_dipole
 *
 * @par Summary
 * Demonstrates the minimal workflow to:
 * - define and parse module options,
 * - construct \ref GMagneto::GMagneto "GMagneto()",
 * - access a field by name via \ref GMagneto::getField "getField()",
 * - and call \ref GField::GetFieldValue "GetFieldValue()" to evaluate the magnetic field vector.
 *
 * This example is referenced from the module mainpage:
 * \ref example_test_gfield_dipole .
 */

// gfields
#include "gfield_options.h"
#include "gmagneto.h"

// c++
#include <iostream>

using namespace std;

/**
 * @brief Entry point of the example program.
 * @param argc Number of command-line arguments.
 * @param argv Command-line argument array.
 * @return Exit status (EXIT_SUCCESS on normal completion).
 *
 * Expected configuration:
 * - The program expects that options define a field named \c "dipole" (for example via \c gmultipoles).
 *
 * Runtime behavior:
 * - Constructs GOptions using gfields::defineOptions().
 * - Constructs \ref GMagneto "GMagneto" which loads and registers configured fields.
 * - If the field exists, evaluates it at 100 random points and discards the results (demonstration only).
 */
int main(int argc, char* argv[]) {

	// Initialize GOptions (parsed from YAML or another source)
	auto gopts = std::make_shared<GOptions>(argc, argv, gfields::defineOptions());

	// Create a GMagneto instance to manage fields
	auto magneto = std::make_shared<GMagneto>(gopts);

	string field_name = "dipole";

	// Check if a specific field exists
	if (magneto->isField(field_name)) {
		auto dipole_field         = magneto->getField(field_name);
		auto dipole_field_manager = magneto->getFieldMgr(field_name);

		// Non-Doxygen summary:
		// Demonstrate repeated evaluation of the field; this intentionally does not attach the field manager
		// to any volume, as the goal is only to show the API surface.

		// pos is a double[3] representing x,y,z
		// iterate through 100 pos that span random positions between 0 and 100 in each coordinate
		for (int i = 0; i < 100; i++) {
			double x = rand() % 100;
			double y = rand() % 100;
			double z = rand() % 100;

			double pos[3] = {x, y, z};
			double bfield[3];
			dipole_field->GetFieldValue(pos, bfield);
		}
	} else {
		cout << "Field " << field_name << " was not found." << endl;
	}

	return EXIT_SUCCESS;
}
