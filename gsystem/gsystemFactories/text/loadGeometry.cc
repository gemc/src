/**
* \file loadGeometry.cc
 * \brief Geometry loading implementation for GSystemTextFactory.
 *
 * See systemTextFactory.h for API docs.
 */

// gsystem
#include "systemTextFactory.h"

// glibrary
#include "gutilities.h"

// c++
#include <iostream>

void GSystemTextFactory::loadGeometry(GSystem* system) {
	// Will exit if not found unless the system annotation allows skipping.
	auto IN = gSystemTextFileStream(system, GTEXTGEOMTYPE);

	if (IN != nullptr) {
		log->info(1, "Loading geometry for system ", system->getName(),
		          " using factory ", system->getFactoryName()
		);

		// Each non-empty line is a serialized parameter row separated by '|'.
		while (!IN->eof()) {
			std::string dbline;
			getline(*IN, dbline);

			if (dbline.empty())
				continue;

			// Extract gvolume parameters.
			std::vector<std::string> gvolumePars =
				gutilities::getStringVectorFromStringWithDelimiter(dbline, "|");
			system->addGVolume(gvolumePars);
		}

		IN->close();
	}
}
