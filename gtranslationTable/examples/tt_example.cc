/**
 * \file tt_example.cc
 * \brief Example program demonstrating basic usage of GTranslationTable.
 *
 * This example shows how to:
 * - Initialize the project options object with the Translation Table module options.
 * - Create a module logger instance using the module logger name.
 * - Construct a GTranslationTable bound to the same options.
 * - Register multiple identities and associated electronics configurations.
 * - Retrieve a configuration by identity and print it.
 *
 * Expected behavior:
 * - Two identities are inserted into the translation table.
 * - One identity is retrieved and printed at verbosity level 0.
 * - If retrieval fails, the module will log an error and return a default-constructed GElectronic.
 *
 * \note This example is intentionally minimal and focuses on the public API:
 *       \ref GTranslationTable::addGElectronicWithIdentity "addGElectronicWithIdentity()"
 *       and \ref GTranslationTable::getElectronics "getElectronics()".
 */

// translationTable
#include "gtranslationTable.h"
#include "gtranslationTable_options.h"

// gemc
#include "glogger.h"

using std::vector;

/**
 * \brief Entry point for the Translation Table example.
 *
 * The program constructs an options object, configures logging, registers two electronics
 * configurations using two distinct identity vectors, retrieves one configuration, and prints it.
 *
 * \param argc Number of command-line arguments.
 * \param argv Command-line argument values.
 * \return \c EXIT_SUCCESS on normal completion.
 */
int main(int argc, char* argv[]) {
	// Build the module options and bind them to the project options infrastructure.
	auto gopts = std::make_shared<GOptions>(argc, argv, gtranslationTable::defineOptions());

	// Create a logger instance for this example.
	// Note: this logger is separate from the logger used internally by GTranslationTable via GBase.
	auto log = std::make_shared<GLogger>(gopts, SFUNCTION_NAME, TRANSLATIONTABLE_LOGGER); // duplicate logger

	// Two distinct identity vectors. In realistic scenarios these may represent detector/channel addressing.
	vector<int> element1 = {1, 2, 3, 4, 5};
	vector<int> element2 = {2, 2, 3, 4, 5};

	// Two example electronics configurations to associate with the identities.
	GElectronic crate1(2, 1, 3, 2);
	GElectronic crate2(2, 1, 4, 2);

	// Construct the translation table. It will use the same options object for its internal logger setup.
	GTranslationTable translationTable(gopts);

	// Register both electronics configurations.
	translationTable.addGElectronicWithIdentity(element1, crate1);
	translationTable.addGElectronicWithIdentity(element2, crate2);

	// Retrieve one configuration and print it.
	GElectronic retrievedElectronic = translationTable.getElectronics(element1);

	// Level 0: essential output for a user running the example.
	log->info(0, "Retrieved electronic: ", retrievedElectronic);

	return EXIT_SUCCESS;
}
