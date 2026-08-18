/**
 * \file gvolume.cc
 * \brief Implementation of GVolume.
 *
 * See gvolume.h for API docs.
 */

// gsystem
#include "gvolume.h"

#include <algorithm>
#include <cctype>
#include <optional>
#include <utility>
#include "gsystemConventions.h"

using namespace gutilities;

namespace {
std::optional<std::string> normalize_optional_geometry_field(std::optional<std::string> value,
                                                             bool remove_all_spaces) {
	if (!value) return std::nullopt;

	std::string trimmed = removeLeadingAndTrailingSpacesFromString(*value);
	if (is_unset(trimmed)) return std::nullopt;

	std::string lowercase = trimmed;
	std::transform(lowercase.begin(), lowercase.end(), lowercase.begin(),
	               [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
	if (lowercase == "no" || lowercase == "none" || lowercase == "not provided") return std::nullopt;

	if (remove_all_spaces) return removeAllSpacesFromString(trimmed);
	return trimmed;
}
} // namespace

// need to set pCopyNo with unique identifier
// see c++ thread safe ID generation function
GVolume::GVolume(const std::shared_ptr<GLogger>& logger,
                 const std::string&              s,
                 std::vector<std::string>        pars,
                 std::optional<std::string>      importPath) :
	GBase(logger),
	system(s),
	importFilename(normalize_optional_geometry_field(std::move(importPath), false)) {
	if (pars.size() == GVOLUMELEGACYNUMBEROFPARS) {
		pars.insert(pars.begin() + 7, DEFAULTG4PLACEMENTTYPE);
	}

	if (pars.size() != GVOLUMENUMBEROFPARS) {
		// Dump received parameters to help diagnose mismatched schema or input corruption.
		for (auto& parameter : pars) { log->warning(" - parameter ", parameter); }

		log->error(ERR_GWRONGNUMBEROFPARS,
		           "Incorrect number of system parameters for GVolume: ", pars.size(), ", it should be ",
		           GVOLUMENUMBEROFPARS);
	}
	else {
		// The parameter vector is a serialized DB/ASCII row. Parsing is positional.
		int i = 0;

		name = removeAllSpacesFromString(pars[i++]);

		// checking that name does not contain GSYSTEM_DELIMITER
		// because GSYSTEM_DELIMITER is used later to build fully-qualified Geant4 names.
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
		g4placementType = removeAllSpacesFromString(pars[i++]);
		if (g4placementType == "" || g4placementType == UNINITIALIZEDSTRINGQUANTITY) {
			g4placementType = DEFAULTG4PLACEMENTTYPE;
		}
		emfield        = normalize_optional_geometry_field(pars[i++], true);
		string pvis    = removeAllSpacesFromString(pars[i++]);
		visible        = (pvis == "1") ? true : false;
		style          = stoi(removeAllSpacesFromString(pars[i++]));
		color          = removeAllSpacesFromString(pars[i++]);
		opacity        = stod(removeAllSpacesFromString(pars[i++]));
		digitization   = normalize_optional_geometry_field(pars[i++], true);
		gidentity      = normalize_optional_geometry_field(pars[i++], false);
		copyOf         = normalize_optional_geometry_field(pars[i++], false);
		solidsOpr      = normalize_optional_geometry_field(pars[i++], false);
		mirror         = normalize_optional_geometry_field(pars[i++], false);
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
	else if (gVol.style == 2) { style = "cloud"; }
	string visibility = "yes";
	if (!gVol.visible) { visibility = "no"; }

	stream << std::endl;
	stream << "   - Name:            " << gVol.name << "  -  " << gVol.description << std::endl;
	stream << "   - System:         " << gVol.system << std::endl;
	stream << "   - Variation:       " << gVol.variation << std::endl;
	stream << "   - Run Number:      " << gVol.runno << std::endl;
	if (gVol.copyOf) stream << "   - copyOf:          " << *gVol.copyOf << std::endl;
	if (gVol.solidsOpr) stream << "   - solidsOpr:       " << *gVol.solidsOpr << std::endl;
	if (gVol.type != "" && gVol.type != UNINITIALIZEDSTRINGQUANTITY)
		stream << "   - Type:            " << gVol.type <<
			std::endl;
	if (gVol.parameters != "" && gVol.parameters != UNINITIALIZEDSTRINGQUANTITY)
		stream << "   - Parameters:      " <<
			gVol.parameters << std::endl;
	stream << "   - Material:        " << gVol.material << std::endl;
	stream << "   - Mother:          " << gVol.motherName << std::endl;
	stream << "   - Positions:       " << gVol.pos << std::endl;
	stream << "   - Rotation(s):     " << gVol.rot << std::endl;
	stream << "   - G4 Placement:    " << gVol.g4placementType << std::endl;
	if (gVol.emfield) stream << "   - E.M. field:      " << *gVol.emfield << std::endl;
	if (gVol.digitization) stream << "   - Digitization:    " << *gVol.digitization << std::endl;
	if (gVol.gidentity) stream << "   - GIdentity:       " << *gVol.gidentity << std::endl;
	stream << "   - Col, Vis, Style: " << gVol.color << ", " << visibility << ", " << style << std::endl;
	stream << std::endl;

	return stream;
}


GVolume::GVolume(const std::string& rootVolumeDefinition,
                 const std::shared_ptr<GLogger>& logger) :
	GBase(logger) {
	// The ROOT/world definition is tokenized by spaces:
	// <solidType> <dim1> <dim2> ... <material>
	vector<string> rootDefinitions = getStringVectorFromStringWithDelimiter(rootVolumeDefinition, " ");
	string         volumeParameters;

	// Build the parameter string (skip the type and the final material token).
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
	g4placementType = DEFAULTG4PLACEMENTTYPE;
	visible      = false;
	style        = 0; // wireframe
	color        = "ccffff";
	exist        = true;

	description = "root volume";

	// modifiers - accessed through options/jcard
	shift = GSYSTEMNOMODIFIER;
	tilt  = GSYSTEMNOMODIFIER;
}

void GVolume::setDigitization(std::optional<std::string> value) {
	digitization = normalize_optional_geometry_field(std::move(value), true);
}

void GVolume::setGIdentity(std::optional<std::string> value) {
	gidentity = normalize_optional_geometry_field(std::move(value), false);
}
