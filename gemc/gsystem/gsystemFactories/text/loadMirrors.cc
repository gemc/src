/**
* \file loadMirrors.cc
 * \brief Mirrors loading implementation for GSystemTextFactory.
 *
 * See systemTextFactory.h for API docs.
 */

// gsystem
#include "systemTextFactory.h"

void GSystemTextFactory::loadMirrors(GSystem* system) {
	// Mirrors file is optional; if missing, IN is nullptr and we simply skip.
	auto IN = gSystemTextFileStream(system, GTEXTMIRSTYPE);

	if (IN != nullptr) {
		log->info(1, "Loading mirrors for system ", system->getName(),
		          " using factory ", system->getFactoryName());

		// Each non-empty line is a serialized parameter row separated by '|'.
		while (!IN->eof()) {
			std::string dbline;
			getline(*IN, dbline);

			if (dbline.empty()) { continue; }

			// Extract gmirror parameters.
			std::vector<std::string> gmirrorsPars =
				gutilities::getStringVectorFromStringWithDelimiter(dbline, "|");
			system->addGMirror(gmirrorsPars);
		}
		// IN (unique_ptr<ifstream>) closes the file and frees the stream on scope exit.
	}
}
