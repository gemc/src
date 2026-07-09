/**
 * \file gmirror.cc
 * \brief Implementation of GMirror.
 *
 * See gmirror.h for API docs.
 */

#include "gutilities.h"

// gsystem
#include "gmirror.h"
#include "gsystemConventions.h"

// c++
#include <sstream>

namespace {
// Unset markers: pygemc writes SQL NULLs ("NULL" once serialized); legacy GEMC2
// sources used "none" / "notDefined".
bool mirror_field_is_unset(const std::string& value) {
	std::string trimmed = gutilities::removeLeadingAndTrailingSpacesFromString(value);
	return gutilities::is_unset(trimmed) || trimmed == "none" || trimmed == "notDefined";
}
}

GMirror::GMirror(const std::string& s, std::vector<std::string> pars,
                 const std::shared_ptr<GLogger>& logger) : GBase(logger),
                                                           system(s) {
	if (pars.size() != GMIRRORNUMBEROFPARS) {
		log->error(ERR_GWRONGNUMBEROFPARS,
		           "Incorrect number of mirror parameters for ", pars[0], ". Expected ",
		           GMIRRORNUMBEROFPARS, " but we got ", pars.size());
	}

	// The parameter vector is a serialized DB/ASCII row. Parsing is positional.
	size_t i = 0;

	name        = gutilities::removeAllSpacesFromString(pars[i++]);
	description = gutilities::removeLeadingAndTrailingSpacesFromString(pars[i++]);

	// Geant4 surface configuration: all mandatory.
	type   = gutilities::removeAllSpacesFromString(pars[i++]);
	finish = gutilities::removeAllSpacesFromString(pars[i++]);
	model  = gutilities::removeAllSpacesFromString(pars[i++]);
	border = gutilities::removeAllSpacesFromString(pars[i++]);

	for (const auto& [field, value] : std::initializer_list<std::pair<const char*, const std::string&>>{
		     {"type", type}, {"finish", finish}, {"model", model}, {"border", border}}) {
		if (mirror_field_is_unset(value)) {
			log->error(ERR_GMIRRORINVALID, "mirror <", name, ">: mandatory field <", field, "> is not set");
		}
	}

	// Boundary optical properties: a material name or the explicit tables below.
	matOptProps = gutilities::removeAllSpacesFromString(pars[i++]);
	if (mirror_field_is_unset(matOptProps)) { matOptProps.clear(); }

	getMirrorPropertyFromString(pars[i++], "photonEnergy");
	getMirrorPropertyFromString(pars[i++], "indexOfRefraction");
	getMirrorPropertyFromString(pars[i++], "reflectivity");
	getMirrorPropertyFromString(pars[i++], "efficiency");
	getMirrorPropertyFromString(pars[i++], "specularlobe");
	getMirrorPropertyFromString(pars[i++], "specularspike");
	getMirrorPropertyFromString(pars[i++], "backscatter");
	getMirrorPropertyFromString(pars[i++], "transmittance");

	std::string sigmaAlphaPar = gutilities::removeLeadingAndTrailingSpacesFromString(pars[i++]);
	// GEMC2 used -1 to mark an unset sigmaAlpha; treat it as unset for compatibility.
	if (!mirror_field_is_unset(sigmaAlphaPar) && sigmaAlphaPar != "-1") {
		try {
			sigmaAlpha    = std::stod(sigmaAlphaPar);
			sigmaAlphaSet = true;
		}
		catch (const std::exception&) {
			log->error(ERR_GMIRRORINVALID, "mirror <", name, ">: could not parse sigmaAlpha <", sigmaAlphaPar, ">");
		}
	}

	// The boundary needs optical properties from one of the two sources.
	if (matOptProps.empty() && photonEnergy.empty()) {
		log->error(ERR_GMIRRORINVALID, "mirror <", name,
		           ">: no optical properties. Set matOptProps or photonEnergy with the properties tables");
	}

	// Every provided property vector must be evaluated on the photonEnergy grid.
	for (const auto& [property, values] : std::initializer_list<std::pair<const char*, const std::vector<double>&>>{
		     {"indexOfRefraction", indexOfRefraction}, {"reflectivity", reflectivity},
		     {"efficiency", efficiency}, {"specularlobe", specularlobe},
		     {"specularspike", specularspike}, {"backscatter", backscatter},
		     {"transmittance", transmittance}}) {
		if (!values.empty() && values.size() != photonEnergy.size()) {
			log->error(ERR_GMIRRORINVALID, "mirror <", name, ">: property <", property, "> has ",
			           values.size(), " entries but photonEnergy has ", photonEnergy.size());
		}
	}
}

bool GMirror::isSkinSurface() const { return border == GMIRRORSKINSURFACE; }

void GMirror::getMirrorPropertyFromString(const std::string& parameter, const std::string& propertyName) {
	// Nothing to do if the parameter is not assigned.
	if (mirror_field_is_unset(parameter)) { return; }

	// Tokenize the string and parse each component to a numeric value with units.
	std::stringstream parameterComponents(parameter);

	while (!parameterComponents.eof()) {
		std::string component;
		parameterComponents >> component;

		std::string trimmedComponent = gutilities::removeLeadingAndTrailingSpacesFromString(component);
		if (trimmedComponent.empty()) { continue; }

		if (propertyName == "photonEnergy") { photonEnergy.push_back(gutilities::getG4Number(trimmedComponent)); }
		else if (propertyName == "indexOfRefraction") {
			indexOfRefraction.push_back(gutilities::getG4Number(trimmedComponent));
		}
		else if (propertyName == "reflectivity") { reflectivity.push_back(gutilities::getG4Number(trimmedComponent)); }
		else if (propertyName == "efficiency") { efficiency.push_back(gutilities::getG4Number(trimmedComponent)); }
		else if (propertyName == "specularlobe") { specularlobe.push_back(gutilities::getG4Number(trimmedComponent)); }
		else if (propertyName == "specularspike") {
			specularspike.push_back(gutilities::getG4Number(trimmedComponent));
		}
		else if (propertyName == "backscatter") { backscatter.push_back(gutilities::getG4Number(trimmedComponent)); }
		else if (propertyName == "transmittance") {
			transmittance.push_back(gutilities::getG4Number(trimmedComponent));
		}
	}
}

std::ostream& operator<<(std::ostream& stream, const GMirror& gMir) {
	stream << std::endl;
	stream << "   - Mirror: " << gMir.name << "  in system  " << gMir.system << ": " << std::endl;
	stream << "     Type / Finish / Model: " << gMir.type << " / " << gMir.finish << " / " << gMir.model << std::endl;
	stream << "     Border:           " << gMir.border << std::endl;
	if (!gMir.matOptProps.empty()) {
		stream << "     Optical properties from material: " << gMir.matOptProps << std::endl;
	}
	else { stream << "     Optical properties at " << gMir.photonEnergy.size() << " photon energies" << std::endl; }
	if (gMir.sigmaAlphaSet) { stream << "     Sigma Alpha:      " << gMir.sigmaAlpha << std::endl; }
	stream << "     Description: " << gMir.description << std::endl;
	stream << std::endl;

	return stream;
}
