// gsystem
#include "systemCadFactory.h"

// c++
#include <filesystem>

using namespace std;

void GSystemCADFactory::loadGeometry(GSystem *s, int verbosity) {
    string dirLocation = searchForDirInLocations(s->getFilePath(), possibleLocationOfFiles);

    if (filesystem::exists(dirLocation)) {
        vector <string> cadFiles = getListOfFilesInDirectory(dirLocation, {".stl"});

        for (auto cf: cadFiles) {
            s->addVolumeFromFile(GSYSTEMCADTFACTORYLABEL, dirLocation + "/" + cf, verbosity);
        }

        // if the file cad__<variation>.yaml is found in dirLocation, modify the gvolumes accordingly
        string cad_yaml = dirLocation + "/cad__" + s->getVariation() + ".yaml";

        if (filesystem::exists(cad_yaml)) {
            cout << GSYSTEMLOGHEADER << "Loading modifiers from YAML file:  " << cad_yaml << endl;

            // parse the YAML file
            YAML::Node ynode = YAML::LoadFile(cad_yaml);

            for (auto &[volumeName, gvolume]: *s->getGVolumesMap()) {
                auto vmod = ynode[volumeName];

                cout << GSYSTEMLOGHEADER << "Looking for cad modifiers for Volume: " << volumeName << ", found: " << vmod << endl;

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


    } else {
        cerr << FATALERRORL << " CAD Directory >" << s->getFilePath() << "< not found." << endl;
        gexit(EC__GDIRNOTFOUND);
    }


}
