/**
* \file loadMaterials.cc
 * \brief Materials loading implementation for GSystemTextFactory.
 *
 * See systemTextFactory.h for API docs.
 */

// gsystem
#include "systemTextFactory.h"

void GSystemTextFactory::loadMaterials(GSystem* system) {
	// Materials file is optional; if missing, IN is nullptr and we simply skip.
	auto IN = gSystemTextFileStream(system, GTEXTMATSTYPE);

	if (IN != nullptr) {
		log->info(1, "Loading materials for system ", system->getName(),
		          " using factory ", system->getFactoryName());

		// Each non-empty line is a serialized parameter row separated by '|'.
		while (!IN->eof()) {
			std::string dbline;
			getline(*IN, dbline);

			if (dbline.empty()) { continue; }

			// Extract gmaterial parameters.
			std::vector<std::string> gmaterialsPars =
				gutilities::getStringVectorFromStringWithDelimiter(dbline, "|");
			system->addGMaterial(gmaterialsPars);
		}

		IN->close();
	}
}
