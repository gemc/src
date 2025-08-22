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

		for (const auto& cf : cadFiles) { s->addVolumeFromFile(GSYSTEMCADTFACTORYLABEL, dirLocation.append("/").append(cf)); }

		// if the file cad__<variation>.yaml is found in dirLocation, modify the gvolumes accordingly
		string cad_yaml = dirLocation + "/cad__" + s->getVariation() + ".yaml";

		if (filesystem::exists(cad_yaml)) {
			log->info(0, "Loading modifiers from YAML file:  " + cad_yaml);

			// parse the YAML file
			YAML::Node ynode = YAML::LoadFile(cad_yaml);

			for (auto& [volumeName, gvolume] : s->getGVolumesMap()) {
				auto vmod = ynode[volumeName];
				log->info(0, "Looking for cad modifiers for Volume: ", volumeName, ", found: ", vmod);

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
