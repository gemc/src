// gsystem
#include "systemTextFactory.h"

void GSystemTextFactory::loadMaterials(GSystem* system) {

	// will continue if not found
	auto IN = gSystemTextFileStream(system, GTEXTMATSTYPE);

	if (IN != nullptr) {
		log->info(1, "Loading materials for system ", system->getName(),
		          " using factory ", system->getFactoryName());

		// loading volumes
		while (!IN->eof()) {
			std::string dbline;
			getline(*IN, dbline);

			if (dbline.empty()) { continue; }

			// extract gvolume parameters
			std::vector<std::string> gmaterialsPars =
				gutilities::getStringVectorFromStringWithDelimiter(dbline, "|");
			system->addGMaterial(gmaterialsPars);
		}

		IN->close();
	}
}
