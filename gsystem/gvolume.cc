// gsystem
#include "gvolume.h"

#include <utility>
#include "gsystemConventions.h"

using namespace gutilities;

// need to set pCopyNo with unique identifier
// see c++ thread safe ID generation function
GVolume::GVolume(const std::shared_ptr<GLogger>& logger,
                 const std::string&              s, vector<string> pars,
                 const std::string&              importPath) :
	GBase(logger),
	system(s),
	importFilename(importPath) {
	if (pars.size() != GVOLUMENUMBEROFPARS) {
		for (auto& parameter : pars) { log->warning(" - parameter ", parameter); }

		log->error(ERR_GWRONGNUMBEROFPARS,
		           "Incorrect number of system parameters for GVolume: ", pars.size(), ", it should be ",
		           GVOLUMENUMBEROFPARS);
	}
	else {
		// size is already checked in addVolume, the only interface to volume
		int i = 0;

		name = removeAllSpacesFromString(pars[i++]);

		// checking that name does not contain GSYSTEM_DELIMITER
		if (name.find(GSYSTEM_DELIMITER) != string::npos) {
			log->error(ERR_GVOLUMENAMECONTAINSINVALID,
			           "the gVolume name <", name, "> contains the invalid character: <", GSYSTEM_DELIMITER,
			           ">. Exiting.");
		}

		type           = removeAllSpacesFromString(pars[i++]);
		parameters     = removeLeadingAndTrailingSpacesFromString(pars[i++]);
		material       = removeAllSpacesFromString(pars[i++]);
		motherName     = removeAllSpacesFromString(pars[i++]);
		pos            = removeLeadingAndTrailingSpacesFromString(pars[i++]);
		rot            = removeLeadingAndTrailingSpacesFromString(pars[i++]);
		emfield        = removeAllSpacesFromString(pars[i++]);
		string pvis    = removeAllSpacesFromString(pars[i++]);
		visible        = (pvis == "1") ? true : false;
		style          = stoi(removeAllSpacesFromString(pars[i++]));
		color          = removeAllSpacesFromString(pars[i++]);
		digitization   = removeAllSpacesFromString(pars[i++]);
		gidentity      = removeLeadingAndTrailingSpacesFromString(pars[i++]);
		copyOf         = removeLeadingAndTrailingSpacesFromString(pars[i++]);
		replicaOf      = removeLeadingAndTrailingSpacesFromString(pars[i++]);
		solidsOpr      = removeLeadingAndTrailingSpacesFromString(pars[i++]);
		mirror         = removeLeadingAndTrailingSpacesFromString(pars[i++]);
		string pexists = removeAllSpacesFromString(pars[i++]);
		exist          = (pexists == "1") ? true : false;

		// these will be assigned later
		g4name       = UNINITIALIZEDSTRINGQUANTITY;
		g4motherName = UNINITIALIZEDSTRINGQUANTITY;

		description = removeLeadingAndTrailingSpacesFromString(pars[i++]);
		variation   = removeLeadingAndTrailingSpacesFromString(pars[i++]);
		runno       = stoi(removeAllSpacesFromString(pars[i++]));

		// modifiers - accessed through options/jcard
		shift = GSYSTEMNOMODIFIER;
		tilt  = GSYSTEMNOMODIFIER;
	}
}


std::ostream& operator<<(std::ostream& stream, const GVolume& gVol) {
	string style = "unknown";
	if (gVol.style == 0) { style = "wireframe"; }
	else if (gVol.style == 1) { style = "solid"; }
	string visibility = "yes";
	if (!gVol.visible) { visibility = "no"; }

	stream << std::endl;
	stream << "   - Name:            " << gVol.name << "  -  " << gVol.description << std::endl;
	stream << "   - System:       " << gVol.system << std::endl;
	stream << "   - Variation:       " << gVol.variation << std::endl;
	stream << "   - Run Number:      " << gVol.runno << std::endl;
	stream << "   - Type:            " << gVol.type << std::endl;
	stream << "   - Parameters:      " << gVol.parameters << std::endl;
	stream << "   - Material:        " << gVol.material << std::endl;
	stream << "   - Mother:          " << gVol.motherName << std::endl;
	stream << "   - Positions:       " << gVol.pos << std::endl;
	stream << "   - Rotation(s):     " << gVol.rot << std::endl;
	stream << "   - E.M. Field:      " << gVol.emfield << std::endl;
	stream << "   - Digitization:    " << gVol.digitization << std::endl;
	stream << "   - GIdentity:       " << gVol.gidentity << std::endl;
	stream << "   - Col, Vis, Style: " << gVol.color << ", " << visibility << ", " << style << std::endl;
	stream << std::endl;

	return stream;
}


GVolume::GVolume(const string& rootVolumeDefinition, const std::shared_ptr<GLogger>& logger) : GBase(logger) {
	vector<string> rootDefinitions = getStringVectorFromStringWithDelimiter(rootVolumeDefinition, " ");
	string         volumeParameters;

	for (size_t i = 1; i < rootDefinitions.size() - 1; i++) { volumeParameters += ", " + rootDefinitions[i]; }

	name         = ROOTWORLDGVOLUMENAME;
	system       = ROOTWORLDGVOLUMENAME;
	variation    = "default";
	type         = rootDefinitions[0];
	parameters   = volumeParameters;
	material     = rootDefinitions.back();
	motherName   = MOTHEROFUSALL;
	pos          = DEFAULTPOSITION;
	rot          = DEFAULTROTATION;
	emfield      = UNINITIALIZEDSTRINGQUANTITY;
	visible      = false;
	style        = 0; // wireframe
	color        = "ccffff";
	digitization = UNINITIALIZEDSTRINGQUANTITY;
	gidentity    = UNINITIALIZEDSTRINGQUANTITY;
	copyOf       = UNINITIALIZEDSTRINGQUANTITY;
	replicaOf    = UNINITIALIZEDSTRINGQUANTITY;
	solidsOpr    = UNINITIALIZEDSTRINGQUANTITY;
	mirror       = UNINITIALIZEDSTRINGQUANTITY;
	exist        = true;

	description = "root volume";

	// modifiers - accessed through options/jcard
	shift = GSYSTEMNOMODIFIER;
	tilt  = GSYSTEMNOMODIFIER;

	// set file with its path if it's a CAD/GDML import
	importFilename = "none";
}
