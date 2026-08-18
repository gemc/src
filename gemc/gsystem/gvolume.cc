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
#include <stdexcept>
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

const std::string& GVolume::getG4Name() const {
	if (!g4name) {
		throw std::logic_error("Geant4 name requested before GVolume <" + system + "/" + name + "> was resolved");
	}
	return *g4name;
}

const std::string& GVolume::getG4MotherName() const {
	if (!g4motherName) {
		throw std::logic_error(
			"Geant4 mother name requested before GVolume <" + system + "/" + name + "> was resolved");
	}
	return *g4motherName;
}

// need to set pCopyNo with unique identifier
// see c++ thread safe ID generation function
GVolume::GVolume(const std::shared_ptr<GLogger>& logger,
                 const std::string&              s,
                 std::vector<std::string>        pars,
                 std::optional<std::string>      importPath) :
	GBase(logger),
	system(s),
	importFilename(normalize_optional_geometry_field(std::move(importPath), false)) {
	if (pars.size() == gsystem::GVOLUMELEGACYNUMBEROFPARS) {
		pars.insert(pars.begin() + 7, gsystem::DEFAULTG4PLACEMENTTYPE);
	}

	if (pars.size() != gsystem::GVOLUMENUMBEROFPARS) {
		// Dump received parameters to help diagnose mismatched schema or input corruption.
		for (auto& parameter : pars) { log->warning(" - parameter ", parameter); }

		log->error(gsystem::ERR_GWRONGNUMBEROFPARS,
		           "Incorrect number of system parameters for GVolume: ", pars.size(), ", it should be ",
		           gsystem::GVOLUMENUMBEROFPARS);
	}
	else {
		// The parameter vector is a serialized DB/ASCII row. Parsing is positional.
		int i = 0;

		name = removeAllSpacesFromString(pars[i++]);

		// checking that name does not contain gsystem::GSYSTEM_DELIMITER
		// because gsystem::GSYSTEM_DELIMITER is used later to build fully-qualified Geant4 names.
		if (name.find(gsystem::GSYSTEM_DELIMITER) != string::npos) {
			log->error(gsystem::ERR_GVOLUMENAMECONTAINSINVALID,
			           "the gVolume name <", name, "> contains the invalid character: <", gsystem::GSYSTEM_DELIMITER,
			           ">. Exiting.");
		}

		type           = removeAllSpacesFromString(pars[i++]);
		parameters     = normalize_optional_geometry_field(pars[i++], false);
		material       = removeAllSpacesFromString(pars[i++]);
		motherName     = removeAllSpacesFromString(pars[i++]);
		pos            = removeLeadingAndTrailingSpacesFromString(pars[i++]);
		rot            = removeLeadingAndTrailingSpacesFromString(pars[i++]);
		g4placementType = removeAllSpacesFromString(pars[i++]);
		if (g4placementType.empty() || gutilities::is_unset(g4placementType)) {
			g4placementType = gsystem::DEFAULTG4PLACEMENTTYPE;
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

		description = removeLeadingAndTrailingSpacesFromString(pars[i++]);
		variation   = removeLeadingAndTrailingSpacesFromString(pars[i++]);
		runno       = stoi(removeAllSpacesFromString(pars[i++]));

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
	if (!gVol.type.empty() && !gutilities::is_unset(gVol.type))
		stream << "   - Type:            " << gVol.type <<
			std::endl;
	if (gVol.parameters) stream << "   - Parameters:      " << *gVol.parameters << std::endl;
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

	name         = gsystem::ROOTWORLDGVOLUMENAME;
	system       = gsystem::ROOTWORLDGVOLUMENAME;
	variation    = "default";
	type         = rootDefinitions[0];
	parameters   = volumeParameters;
	material     = rootDefinitions.back();
	motherName   = gsystem::MOTHEROFUSALL;
	pos          = gsystem::DEFAULTPOSITION;
	rot          = gsystem::DEFAULTROTATION;
	g4placementType = gsystem::DEFAULTG4PLACEMENTTYPE;
	visible      = false;
	style        = 0; // wireframe
	color        = "ccffff";
	exist        = true;

	description = "root volume";

}

void GVolume::setDigitization(std::optional<std::string> value) {
	digitization = normalize_optional_geometry_field(std::move(value), true);
}

void GVolume::setGIdentity(std::optional<std::string> value) {
	gidentity = normalize_optional_geometry_field(std::move(value), false);
}
