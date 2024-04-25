// gsystem
#include "gsystem.h"
#include "gsystemConventions.h"

// glibrary
#include "gutilities.h"
#include "goptionsConventions.h"

// c++
#include <iostream>
using namespace std;

// init system based on name, factory, variation and run number
GSystem::GSystem(string n, string f, string v, int verbosity, int r, string notes, string sqlf):
factoryName(f),
variation(v),
runno(r),
annotations(notes),
sqlite_file(sqlf)
{
	path = gutilities::getDirFromPath(n);
	name = gutilities::getFileFromPath(n);
	
	// if the name of the system does not have a path
	// getDirFromPath will return the name
	// in that case reset the path to an empty string
	if ( name == path ) {
		path = "";
	}

	gvolumesMap   = new map<string, GVolume*>;
	gmaterialsMap = new map<string, GMaterial*>;

	if(verbosity >= GVERBOSITY_SUMMARY) {
		cout << GSYSTEMLOGHEADER << "Instantiating GSystem <" << KGRN << name  << RST << "> using path <" << KGRN << path  << RST << ">" << endl;
	}
}

// MARK: GVOLUMES

// build and add a gvolume to the map from system parameters
void GSystem::addGVolume(vector<string> pars, int verbosity) {

    pars.push_back(variation);
    pars.push_back(to_string(runno));

	string voumeName = pars[0];

	if(gvolumesMap->find(voumeName) == gvolumesMap->end()) {

		if(verbosity >= GVERBOSITY_DETAILS) {
			cout << GSYSTEMLOGHEADER << "Adding gVolume <" << voumeName << "> to gvolumesMap with name as key"<< endl;
		}
		(*gvolumesMap)[voumeName] = new GVolume(name, pars);

		if(verbosity >= GVERBOSITY_DETAILS) {
			cout << *(*gvolumesMap)[voumeName] ;
		}

	} else {
		cerr << FATALERRORL << "a gVolume with the name <" << voumeName << "> already exists. " << endl;
		gexit(EC__GVOLUMEALREADYPRESENT);
	}

}

void GSystem::addROOTVolume(string rootVolumeDefinition) {
	// special constructor for root volume
	(*gvolumesMap)[ROOTWORLDGVOLUMENAME] = new GVolume(rootVolumeDefinition);
}

// add volume from file (CAD or GDML factories)
// includes factory and filename in the definitions
#include <filesystem>
namespace fs = std::filesystem;
void GSystem::addVolumeFromFile(string importType, string filename, int verbosity) {

	vector<string> pars;

	vector<string> gvpaths = getStringVectorFromStringWithDelimiter( fs::path(filename).filename(), ".");

	// first, get fist item w/o path
	string gvolumeName = gvpaths.front();

	// order is defined in gvolume.cc
	pars.push_back(gvolumeName);                           // 01 name
	pars.push_back(importType);                            // 03 type
	pars.push_back(UNINITIALIZEDSTRINGQUANTITY);           // 04 parameters
	pars.push_back("G4_AIR");                              // 05 material: default is air
    pars.push_back(ROOTWORLDGVOLUMENAME);                  // 02 mother: by default is ROOTWORLDGVOLUMENAME
	pars.push_back("0*cm, 0*cm, 0*cm");                    // 06 pos
	pars.push_back("0*deg, 0*deg, 0*deg");                 // 07 rot
	pars.push_back(UNINITIALIZEDSTRINGQUANTITY);           // 08 emfield
	pars.push_back("1");                                   // 09 visible
	pars.push_back("1");                                   // 10 style
	pars.push_back("999999");                              // 11 color
	pars.push_back(UNINITIALIZEDSTRINGQUANTITY);           // 12 digitization
	pars.push_back(UNINITIALIZEDSTRINGQUANTITY);           // 13 gidentity
	pars.push_back(UNINITIALIZEDSTRINGQUANTITY);           // 14 copyOf
	pars.push_back(UNINITIALIZEDSTRINGQUANTITY);           // 15 replicaOf
	pars.push_back(UNINITIALIZEDSTRINGQUANTITY);           // 16 solidsOpr
	pars.push_back(UNINITIALIZEDSTRINGQUANTITY);           // 17 mirrot
	pars.push_back("1");                                   // 18 exist
	pars.push_back(filename);                              // 19 description: contains full path

	addGVolume(pars, verbosity);
}


// need to filter system name from key
GVolume* GSystem::getGVolume(string volumeName) const {

	if(gvolumesMap->find(volumeName) != gvolumesMap->end()) {
		return (*gvolumesMap)[volumeName];
	} else {
		return nullptr;
	}
}

// MARK: GMATERIALS

// add gmaterial using parameters (TEXT or SQL factories)
void GSystem::addGMaterial(vector<string> pars, int verbosity) {

	string materialName = pars[0];

	if(gmaterialsMap->find(materialName) == gmaterialsMap->end()) {

		(*gmaterialsMap)[materialName] = new GMaterial(name, pars);

		if(verbosity >= GVERBOSITY_SUMMARY) {
			cout << GSYSTEMLOGHEADER << "Adding gMaterial <" << materialName << ">" << endl;
		}
		if(verbosity >= GVERBOSITY_DETAILS) {
			cout << *(*gmaterialsMap)[materialName] ;
		}

	} else {
		cerr << FATALERRORL << "a gMaterial with the name <" << materialName << "> already exists. " << endl;
		gexit(EC__GVOLUMEALREADYPRESENT);
	}


}

const GMaterial* GSystem::getMaterialForGVolume(string volumeName) const {

	if( gvolumesMap->find(volumeName) != gvolumesMap->end() ) {
		string materialName= (*gvolumesMap)[volumeName]->getMaterial();

		if( gmaterialsMap->find(materialName) != gmaterialsMap->end() ) {
			return  (*gmaterialsMap)[materialName];
		} else {
			cerr << FATALERRORL << "gMaterial <" << materialName << "> not found for volume <" << volumeName << ">" << endl;
			gexit(EC__GMATERIALNOTFOUND);
		}
	}

	return nullptr;
}
