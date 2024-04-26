// gsystem
#include "systemCadFactory.h"

// c++
#include <filesystem>
using namespace std;

void GSystemCADFactory::loadGeometry(GSystem *s, int verbosity)
{
	string dirLocation = searchForDirInLocations(s->getFilePath(), possibleLocationOfFiles);

	if ( filesystem::exists(dirLocation) ) {
		vector<string> cadFiles = getListOfFilesInDirectory(dirLocation, {".stl"});

		for(auto cf: cadFiles) {
			s->addVolumeFromFile(GSYSTEMCADTFACTORYLABEL, dirLocation + "/" + cf, verbosity);
		}

        // if the file cad__<variation>.json is found in dirLocation, modify the gvolumes accordingly
        string cad_json = dirLocation + "/cad__" + s->getVariation() + ".json";
        if ( filesystem::exists(cad_json) ) {
            cout << GSYSTEMLOGHEADER << "Loading modifiers from " << cad_json << endl;
            std::ifstream f(cad_json);
            json modifiers = json::parse(f);

            for(auto& [volumeName, gvolume] : *s->getGVolumesMap() ) {
                auto vmod = modifiers[volumeName];
                if ( vmod != nullptr ) {
                    auto shift = vmod["shift"];
                    if ( shift != nullptr ) {
                        gvolume->applyShift(shift);
                    }
                    auto tilt = vmod["tilt"];
                    if ( tilt != nullptr ) {
                        gvolume->applyTilt(tilt);
                    }
                    auto exists = vmod["exists"];
                    if ( exists != nullptr ) {
                        gvolume->modifyExistence(exists);
                    }
                    auto mother = vmod["mother"];
                    if ( mother != nullptr ) {
                        gvolume->resetMotherName(mother);
                    }
                    auto color = vmod["color"];
                    if ( color != nullptr ) {
                        gvolume->setColor(color);
                    }
                    auto material = vmod["material"];
                    if ( material != nullptr ) {
                        gvolume->setMaterial(material);
                    }
                    auto digitization = vmod["digitization"];
                    if ( digitization != nullptr ) {
                        gvolume->setDigitization(digitization);
                    }
                    auto identifier = vmod["identifier"];
                    if ( identifier != nullptr ) {
                        gvolume->setGIdentity(identifier);
                    }
                }
            }


        }


	} else {
		cerr << FATALERRORL << " CAD Directory >" << s->getFilePath() << "< not found." << endl;
		gexit(EC__GDIRNOTFOUND);
	}




}



