// translationTable
#include "gtranslationTable.h"
#include "gtranslationTable_options.h"

// gemc
#include "glogger.h"

int main(int argc, char *argv[]) {

	GOptions *gopts = new GOptions(argc, argv, gtranslationTable::defineOptions());
	GLogger *log = new GLogger(gopts, TRANSLATIONTABLE_LOGGER);

	vector<int> element1 = {1, 2, 3, 4, 5};
	vector<int> element2 = {2, 2, 3, 4, 5};

	GElectronic crate1(2, 1, 3, 2);
	GElectronic crate2(2, 1, 4, 2);

	GTranslationTable translationTable(log);

	translationTable.addGElectronicWithIdentity(element1, crate1);
	translationTable.addGElectronicWithIdentity(element2, crate2);

	GElectronic retrievedElectronic = translationTable.getElectronics(element1);

	log->info(0, "Retrieved electronic: ", retrievedElectronic);

	// cleanup
	// deleting log here gives error on linux. should be investigated
	delete gopts;

	return EXIT_SUCCESS;
}
