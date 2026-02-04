/**
 * \file gmaterial.cc
 * \brief Implementation of GMaterial.
 *
 * See gmaterial.h for API docs.
 */

#include "gutilities.h"
//using namespace gutilities;


// gsystem
#include "gmaterial.h"
#include "gsystemConventions.h"

// c++
#include <iostream>
#include <sstream>


GMaterial::GMaterial(const std::string& s, std::vector<std::string> pars, const std::shared_ptr<GLogger>& logger) :
	GBase(logger),
	system(s) {
	if (pars.size() != GMATERIALNUMBEROFPARS) {
		log->error(ERR_GWRONGNUMBEROFPARS,
		           "Incorrect number of material parameters for ", pars[0], ". Expected ",
		           GMATERIALNUMBEROFPARS, " but we got ", pars.size());
	}
	else {
		// The parameter vector is a serialized DB/ASCII row. Parsing is positional.
		size_t i = 0;

		name    = gutilities::removeAllSpacesFromString(pars[i++]);
		density = stod(gutilities::removeAllSpacesFromString(pars[i++]));

		// The "composition" field is tokenized into (component, amount) pairs.
		setComponentsFromString(pars[i++]);

		// Human-readable description (kept verbatim).
		description = pars[i++];

		// Optical properties: each field may be UNINITIALIZEDSTRINGQUANTITY, in which case it is skipped.
		getMaterialPropertyFromString(pars[i++], "photonEnergy");
		getMaterialPropertyFromString(pars[i++], "indexOfRefraction");
		getMaterialPropertyFromString(pars[i++], "absorptionLength");
		getMaterialPropertyFromString(pars[i++], "reflectivity");
		getMaterialPropertyFromString(pars[i++], "efficiency");

		// Scintillation properties (spectra first, then scalars).
		getMaterialPropertyFromString(pars[i++], "fastcomponent");
		getMaterialPropertyFromString(pars[i++], "slowcomponent");

		assign_if_set(pars, i, scintillationyield);
		assign_if_set(pars, i, resolutionscale);
		assign_if_set(pars, i, fasttimeconstant);
		assign_if_set(pars, i, slowtimeconstant);
		assign_if_set(pars, i, yieldratio);
		assign_if_set(pars, i, birksConstant);

		// Other optical processes: the final vector load triggers cross-vector size validation.
		getMaterialPropertyFromString(pars[i++], "rayleigh");
	}
}


std::ostream& operator<<(std::ostream& stream, const GMaterial& gMat) {
	stream << std::endl;
	stream << "   - Material: " << gMat.name << "  in system  " << gMat.system << ": " << std::endl;
	stream << "     Density:          " << gMat.density << std::endl;
	if (!gMat.components.empty()) {
		stream << "     Composition:          " << std::endl;
		for (unsigned m = 0; m < gMat.components.size(); m++) {
			std::string quantity = gMat.amounts[m] > 1 ? " atoms " : " fractional mass";
			stream << "       ・ " << gMat.components[m] << quantity << " " << gMat.amounts[m] << std::endl;
		}
	}
	stream << "     Description: " << gMat.description << std::endl;
	stream << std::endl;

	return stream;
}


// sets components and amounts
void GMaterial::setComponentsFromString(const std::string& composition) {
	std::vector<std::string> allComponents = gutilities::getStringVectorFromString(composition);

	// Interpret alternating tokens as (component, amount) pairs.
	for (unsigned e = 0; e < allComponents.size() / 2; e++) {
		components.push_back(allComponents[e * 2]);
		amounts.push_back(stod(allComponents[e * 2 + 1]));
	}
}


// load property from DB entry based on its name
void GMaterial::getMaterialPropertyFromString(const std::string& parameter, const std::string& propertyName) {
	// Nothing to do if the parameter is not assigned.
	if (gutilities::removeLeadingAndTrailingSpacesFromString(parameter) == UNINITIALIZEDSTRINGQUANTITY) { return; }

	// Tokenize the string and parse each component to a numeric value with units.
	std::stringstream parameterComponents(parameter);

	while (!parameterComponents.eof()) {
		std::string component;
		parameterComponents >> component;

		// Removing whitespaces.
		std::string trimmedComponent = gutilities::removeLeadingAndTrailingSpacesFromString(component);

		// Vector-valued properties append one value per token.
		if (propertyName == "photonEnergy") { photonEnergy.push_back(gutilities::getG4Number(trimmedComponent)); }
		else if (propertyName == "indexOfRefraction") {
			indexOfRefraction.push_back(gutilities::getG4Number(trimmedComponent));
		}
		else if (propertyName == "absorptionLength") {
			absorptionLength.push_back(gutilities::getG4Number(trimmedComponent));
		}
		else if (propertyName == "reflectivity") { reflectivity.push_back(gutilities::getG4Number(trimmedComponent)); }
		else if (propertyName == "efficiency") { efficiency.push_back(gutilities::getG4Number(trimmedComponent)); }
		else if (propertyName == "fastcomponent") {
			fastcomponent.push_back(gutilities::getG4Number(trimmedComponent));
		}
		else if (propertyName == "slowcomponent") {
			slowcomponent.push_back(gutilities::getG4Number(trimmedComponent));
		}
		// Scalar-valued properties overwrite with the last parsed token.
		else if (propertyName == "scintillationyield") {
			scintillationyield = gutilities::getG4Number(trimmedComponent);
		}
		else if (propertyName == "resolutionscale") { resolutionscale = gutilities::getG4Number(trimmedComponent); }
		else if (propertyName == "fasttimeconstant") { fasttimeconstant = gutilities::getG4Number(trimmedComponent); }
		else if (propertyName == "slowtimeconstant") { slowtimeconstant = gutilities::getG4Number(trimmedComponent); }
		else if (propertyName == "yieldratio") { yieldratio = gutilities::getG4Number(trimmedComponent); }
		else if (propertyName == "birkConstant") { birksConstant = gutilities::getG4Number(trimmedComponent); }
		else if (propertyName == "rayleigh") { rayleigh.push_back(gutilities::getG4Number(trimmedComponent)); }


		if (propertyName == "rayleigh") {
			// Rayleigh is loaded last: at this point we can validate that all other optical vectors
			// either are empty (not specified) or match the photonEnergy vector length.
			//
			// If they do not match, behavior is undefined because properties would be evaluated
			// at inconsistent energy grids.
			unsigned long photonEnergyVectorSize = photonEnergy.size();

			if (!indexOfRefraction.empty() && indexOfRefraction.size() != photonEnergyVectorSize) {
				log->error(ERR_GMATERIALOPTICALPROPERTYMISMATCH,
				           "indexOfRefraction size ", indexOfRefraction.size(), " mismatch: photonEnergy has size ",
				           photonEnergyVectorSize);
			}
			if (!absorptionLength.empty() && absorptionLength.size() != photonEnergyVectorSize) {
				log->error(ERR_GMATERIALOPTICALPROPERTYMISMATCH,
				           "absorptionLength size ", absorptionLength.size(), " mismatch: photonEnergy has size ",
				           photonEnergyVectorSize);
			}
			if (!reflectivity.empty() && reflectivity.size() != photonEnergyVectorSize) {
				log->error(ERR_GMATERIALOPTICALPROPERTYMISMATCH,
				           "reflectivity size ", reflectivity.size(), " mismatch: photonEnergy has size ",
				           photonEnergyVectorSize);
			}
			if (!efficiency.empty() && efficiency.size() != photonEnergyVectorSize) {
				log->error(ERR_GMATERIALOPTICALPROPERTYMISMATCH,
				           "efficiency size ", efficiency.size(), " mismatch: photonEnergy has size ",
				           photonEnergyVectorSize);
			}
			if (!fastcomponent.empty() && fastcomponent.size() != photonEnergyVectorSize) {
				log->error(ERR_GMATERIALOPTICALPROPERTYMISMATCH,
				           "fastcomponent size ", fastcomponent.size(), " mismatch: photonEnergy has size ",
				           photonEnergyVectorSize);
			}
			if (!slowcomponent.empty() && slowcomponent.size() != photonEnergyVectorSize) {
				log->error(ERR_GMATERIALOPTICALPROPERTYMISMATCH,
				           "slowcomponent size ", slowcomponent.size(), " mismatch: photonEnergy has size ",
				           photonEnergyVectorSize);
			}
			if (!rayleigh.empty() && rayleigh.size() != photonEnergyVectorSize) {
				log->error(ERR_GMATERIALOPTICALPROPERTYMISMATCH,
				           "rayleigh size ", rayleigh.size(), " mismatch: photonEnergy has size ",
				           photonEnergyVectorSize);
			}
		}
	}
}


bool GMaterial::assign_if_set(const std::vector<std::string>& pars, size_t& i, double& out) {
	if (i >= pars.size()) return false;

	const std::string trimmed =
		gutilities::removeLeadingAndTrailingSpacesFromString(pars[i++]); // owns storage

	std::string_view sv(trimmed);
	if (gutilities::is_unset(sv)) return false;

	try {
		out = std::stod(trimmed);
		return true;
	}
	catch (const std::exception&) {
		// Do not error here: malformed optional scalars are treated as "not set".
		return false;
	}
}
