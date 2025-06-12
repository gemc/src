// string for gexit
#include "gutilities.h"

// gsystem
#include "systemTextFactory.h"
#include "gsystemConventions.h"

// cpp
#include <fstream>

GSystemTextFactory::GSystemTextFactory() = default;

// returns the file stream, checking all possible directories.
// SYSTEMTYPE can be:
// - GTEXTGEOMTYPE (mandatory, exit if not found)
// - GTEXTMATSTYPE
std::ifstream* GSystemTextFactory::gSystemTextFileStream(GSystem* system, const string& SYSTEMTYPE, const std::shared_ptr<GLogger>& log) {
	string fileName  = system->getFilePath();
	string variation = system->getVariation();

	string fname = fileName + SYSTEMTYPE + variation + ".txt";


	log->info(0, "gSystemTextFileStream filename is: ", fname);

	// default dir is "."
	auto IN = new std::ifstream(fname.c_str());

	if (IN->good()) {
		log->info(1, "Trying file ", fname);
		return IN;
	}
	else {
		// file not good, now trying other locations
		for (const auto& trialLocation : possibleLocationOfFiles) {
			string newName = trialLocation + "/" + fname;

			log->info(1, "Trying file ", newName);

			IN->open(newName.c_str());
			if (IN->good()) {
				// file found, return stream
				log->info(1, "Trying file ", newName);
				return IN;
			}
		}
	}


	// at this point file was not found
	// this coulLd be ok if we are looking for the materials file - if the system annotation is 'mats_only'
	if (SYSTEMTYPE == GTEXTGEOMTYPE) {
		if (system->getAnnotations() == "mats_only") {
			log->info(1, "File ", fname, " not found, but this is ok because the system annotation is 'mats_only'");
			return nullptr;
		}
		log->error(ERR_GSETUPFILENOTOFOUND, "File ", fname, " not found.");
	}

	// file was not found
	return nullptr;
}
