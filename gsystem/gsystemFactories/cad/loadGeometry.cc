/**
 * \file loadGeometry.cc
 * \brief Geometry loading implementation for GSystemCADFactory.
 *
 * See systemCadFactory.h for API docs.
 */

// gsystem
#include "systemCadFactory.h"
#include "gsystemConventions.h"

// gemc
#include "gutilities.h"


// c++
#include <filesystem>

using namespace std;

void GSystemCADFactory::loadGeometry(GSystem* s) {
	string dirLocation = gutilities::searchForDirInLocations(s->getFilePath(), possibleLocationOfFiles);

	if (filesystem::exists(dirLocation)) {
		vector<string> cadFiles = gutilities::getListOfFilesInDirectory(dirLocation, {".stl"});

		// Import each STL as a volume. Each volume name is derived from the filename.
		for (const auto& cf : cadFiles) {
			s->addVolumeFromFile(GSYSTEMCADTFACTORYLABEL, dirLocation.append("/").append(cf));
		}

		// If the file cad__<variation>.yaml is found in dirLocation, modify the gvolumes accordingly.
		string cad_yaml = dirLocation + "/cad__" + s->getVariation() + ".yaml";

		if (filesystem::exists(cad_yaml)) {
			log->info(0, "Loading modifiers from YAML file:  " + cad_yaml);

			// Parse the YAML file.
			// YAML types are external; use \c in documentation (this code is intentionally unchanged).
			YAML::Node ynode = YAML::LoadFile(cad_yaml);

			for (auto& [volumeName, gvolume] : s->getGVolumesMap()) {
				// Each top-level YAML key is expected to match a volume name.
				auto vmod = ynode[volumeName];
				log->info(0, "Looking for cad modifiers for Volume: ", volumeName, ", found: ", vmod);

				// The following commented block shows the intended application of YAML overrides
				// to already-imported volumes (shift/tilt/existence/mother/color/material/etc.).
				//  if (vmod != nullptr) {
				//                    auto shift = vmod["shift"];
				//                    if (shift != nullptr) {
				//                        gvolume->applyShift(shift);
				//                    }
				//                    auto tilt = vmod["tilt"];
				//                    if (tilt != nullptr) {
				//                        gvolume->applyTilt(tilt);
				//                    }
				//                    auto exists = vmod["exists"];
				//                    if (exists != nullptr) {
				//                        gvolume->modifyExistence(exists);
				//                    }
				//                    auto mother = vmod["mother"];
				//                    if (mother != nullptr) {
				//                        gvolume->resetMotherName(mother);
				//                    }
				//                    auto color = vmod["color"];
				//                    if (color != nullptr) {
				//                        gvolume->setColor(color);
				//                    }
				//                    auto material = vmod["material"];
				//                    if (material != nullptr) {
				//                        gvolume->setMaterial(material);
				//                    }
				//                    auto digitization = vmod["digitization"];
				//                    if (digitization != nullptr) {
				//                        gvolume->setDigitization(digitization);
				//                    }
				//                    auto identifier = vmod["identifier"];
				//                    if (identifier != nullptr) {
				//                        gvolume->setGIdentity(identifier);
				//                    }
				//               }
			}
		}
	}
	else {
		log->error(ERR_GDIRNOTFOUND, "CAD Directory >" + s->getFilePath() + "< not found.");
	}
}
