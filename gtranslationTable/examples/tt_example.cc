// translationTable
#include "gtranslationTable.h"
#include "gtranslationTable_options.h"

// gemc
#include "glogger.h"

using std::vector;

int main(int argc, char* argv[]) {
	auto gopts = std::make_shared<GOptions>(argc, argv, gtranslationTable::defineOptions());
	auto log   = std::make_shared<GLogger>(gopts, SFUNCTION_NAME, TRANSLATIONTABLE_LOGGER); // duplicate logger

	vector<int> element1 = {1, 2, 3, 4, 5};
	vector<int> element2 = {2, 2, 3, 4, 5};

	GElectronic crate1(2, 1, 3, 2);
	GElectronic crate2(2, 1, 4, 2);

	GTranslationTable translationTable(gopts);

	translationTable.addGElectronicWithIdentity(element1, crate1);
	translationTable.addGElectronicWithIdentity(element2, crate2);

	GElectronic retrievedElectronic = translationTable.getElectronics(element1);

	log->info(0, "Retrieved electronic: ", retrievedElectronic);

	return EXIT_SUCCESS;
}
