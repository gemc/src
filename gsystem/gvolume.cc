// gsystem
#include "gvolume.h"

#include <utility>
#include "gsystemConventions.h"

using namespace std;
using namespace gutilities;

// need to set pCopyNo with unique identifier
// see c++ thread safe ID generation function
GVolume::GVolume(const std::shared_ptr<GLogger>& log, string s, vector <string> pars, string importPath) : system(std::move(s)) {

    if (pars.size() != GVOLUMENUMBEROFPARS) {

		for (auto &parameter: pars) {
		    cerr << " par " << parameter << endl;
			log->warning(" - parameter ", parameter);
		}

    	log->error(ERR__GWRONGNUMBEROFPARS,
		           "Incorrect number of system parameters for GVolume: ", pars.size(), ", it should be ", GVOLUMENUMBEROFPARS);

    } else {
        // size is already checked in addVolume, the only interface to volume
        int i = 0;

        name = removeAllSpacesFromString(pars[i++]);

        // checking that name does not contain GSYSTEM_DELIMITER
        if (name.find(GSYSTEM_DELIMITER) != string::npos) {

        	log->error(ERR__GVOLUMENAMECONTAINSINVALID,
			           "the gVolume name <", name, "> contains the invalid character: <", GSYSTEM_DELIMITER, ">. Exiting.");
        }

        type = removeAllSpacesFromString(pars[i++]);
        parameters = removeLeadingAndTrailingSpacesFromString(pars[i++]);
        material = removeAllSpacesFromString(pars[i++]);
        motherName = removeAllSpacesFromString(pars[i++]);
        pos = removeLeadingAndTrailingSpacesFromString(pars[i++]);
        rot = removeLeadingAndTrailingSpacesFromString(pars[i++]);
        emfield = removeAllSpacesFromString(pars[i++]);
        string pvis = removeAllSpacesFromString(pars[i++]);
        visible = (pvis == "1") ? true : false;
        style = stoi(removeAllSpacesFromString(pars[i++]));
        color = removeAllSpacesFromString(pars[i++]);
        digitization = removeAllSpacesFromString(pars[i++]);
        gidentity = removeLeadingAndTrailingSpacesFromString(pars[i++]);
        copyOf = removeLeadingAndTrailingSpacesFromString(pars[i++]);
        replicaOf = removeLeadingAndTrailingSpacesFromString(pars[i++]);
        solidsOpr = removeLeadingAndTrailingSpacesFromString(pars[i++]);
        mirror = removeLeadingAndTrailingSpacesFromString(pars[i++]);
        string pexists = removeAllSpacesFromString(pars[i++]);
        exist = (pexists == "1") ? true : false;

        // these will be assigned later
        g4name = UNINITIALIZEDSTRINGQUANTITY;
        g4motherName = UNINITIALIZEDSTRINGQUANTITY;

        description = removeLeadingAndTrailingSpacesFromString(pars[i++]);
        variation = removeLeadingAndTrailingSpacesFromString(pars[i++]);
        runno = stoi(removeAllSpacesFromString(pars[i++]));

        // modifiers - accessed through options/jcard
        shift = GSYSTEMNOMODIFIER;
        tilt = GSYSTEMNOMODIFIER;

        // set file with path if it's a CAD/GDML import
        importFilename = std::move(importPath);
    }
}


ostream &operator<<(ostream &stream, const GVolume& gVol) {
    string style = "unknown";
    if (gVol.style == 0) {
        style = "wireframe";
    } else if (gVol.style == 1) {
        style = "solid";
    }
    string visibility = "yes";
    if (!gVol.visible) {
        visibility = "no";
    }

    stream << endl;
    stream << "   - Name:            " << gVol.name << "  -  " << gVol.description << endl;
    stream << "   - Type:            " << gVol.type << endl;
    stream << "   - Parameters:      " << gVol.parameters << endl;
    stream << "   - Material:        " << gVol.material << endl;
    stream << "   - Mother:          " << gVol.motherName << endl;
    stream << "   - Positions:       " << gVol.pos << endl;
    stream << "   - Rotation(s):     " << gVol.rot << endl;
    stream << "   - E.M. Field:      " << gVol.emfield << endl;
    stream << "   - Digitization:    " << gVol.digitization << endl;
    stream << "   - GIdentity:       " << gVol.gidentity << endl;
    stream << "   - Col, Vis, Style: " << gVol.color << ", " << visibility << ", " << style << endl;
    stream << "   - Variation:       " << gVol.variation << endl;
    stream << "   - Run Number:      " << gVol.runno << endl;
    stream << endl;

    return stream;
}


GVolume::GVolume(const string& rootVolumeDefinition) {

    vector <string> rootDefinitions = getStringVectorFromStringWithDelimiter(rootVolumeDefinition, " ");
    string volumeParameters;

    for (size_t i = 1; i < rootDefinitions.size() - 1; i++) {
        volumeParameters += ", " + rootDefinitions[i];
    }

    name = ROOTWORLDGVOLUMENAME;
    type = rootDefinitions[0];
    parameters = volumeParameters;
    material = rootDefinitions.back();
    motherName = MOTHEROFUSALL;
    pos = DEFAULTPOSITION;
    rot = DEFAULTROTATION;
    emfield = UNINITIALIZEDSTRINGQUANTITY;
    visible = true;
    style = 0; // wireframe
    color = "ccffff";
    digitization = UNINITIALIZEDSTRINGQUANTITY;
    gidentity = UNINITIALIZEDSTRINGQUANTITY;
    copyOf = UNINITIALIZEDSTRINGQUANTITY;
    replicaOf = UNINITIALIZEDSTRINGQUANTITY;
    solidsOpr = UNINITIALIZEDSTRINGQUANTITY;
    mirror = UNINITIALIZEDSTRINGQUANTITY;
    exist = true;

    description = "root volume";

    // modifiers - accessed through options/jcard
    shift = GSYSTEMNOMODIFIER;
    tilt = GSYSTEMNOMODIFIER;

    // set file with path if it's a CAD/GDML import
    importFilename = "none";
}
