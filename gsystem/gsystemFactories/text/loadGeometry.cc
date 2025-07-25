// gsystem
#include "systemTextFactory.h"

// glibrary
#include "gutilities.h"

// c++
#include <iostream>

void GSystemTextFactory::loadGeometry(GSystem* system, std::shared_ptr<GLogger> log) {

	// will exit if not found
	auto IN = gSystemTextFileStream(system, GTEXTGEOMTYPE, log);

	if (IN != nullptr) {
		log->info(1, "Loading geometry for system ", system->getName(),
		          " using factory ", system->getFactoryName()
		         );

		// loading volumes
		while (!IN->eof()) {
			std::string dbline;
			getline(*IN, dbline);

			if (dbline.empty())
				continue;

			// extract gvolume parameters
			std::vector<std::string> gvolumePars =
				gutilities::getStringVectorFromStringWithDelimiter(dbline, "|");
			system->addGVolume(gvolumePars);
		}

		IN->close();
	}
}
