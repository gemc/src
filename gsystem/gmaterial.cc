#include "gutilities.h"
using namespace gutilities;


// gsystem
#include "gmaterial.h"
#include "gsystemConventions.h"

// c++
#include <iostream>
#include <sstream>
#include <utility>

using namespace std;


GMaterial::GMaterial(string s, vector<string> pars, shared_ptr<GLogger> logger) : system(std::move(s)), log(logger) {
	if (pars.size() != GMATERIALNUMBEROFPARS) {
		log->error(ERR_GWRONGNUMBEROFPARS,
		           "Incorrect number of material parameters for ", pars[0], ". Expected ",
		           to_string(GMATERIALNUMBEROFPARS), " but we got ", to_string(pars.size()));
	}
	else {
		// size is already checked in addVolume, the only interface to volume
		int i = 0;

		name    = removeAllSpacesFromString(pars[i++]);
		density = stod(removeAllSpacesFromString(pars[i++]));

		// sets components and amounts
		setComponentsFromString(pars[i++]);

		// description
		description = pars[i++];

		// optical properties
		getMaterialPropertyFromString(pars[i++], "photonEnergy");
		getMaterialPropertyFromString(pars[i++], "indexOfRefraction");
		getMaterialPropertyFromString(pars[i++], "absorptionLength");
		getMaterialPropertyFromString(pars[i++], "reflectivity");
		getMaterialPropertyFromString(pars[i++], "efficiency");

		// scintillation properties
		getMaterialPropertyFromString(pars[i++], "fastcomponent");
		getMaterialPropertyFromString(pars[i++], "slowcomponent");

		scintillationyield = stod(removeLeadingAndTrailingSpacesFromString(pars[i++]));
		resolutionscale    = stod(removeLeadingAndTrailingSpacesFromString(pars[i++]));
		fasttimeconstant   = stod(removeLeadingAndTrailingSpacesFromString(pars[i++]));
		slowtimeconstant   = stod(removeLeadingAndTrailingSpacesFromString(pars[i++]));
		yieldratio         = stod(removeLeadingAndTrailingSpacesFromString(pars[i++]));
		birkConstant       = stod(removeLeadingAndTrailingSpacesFromString(pars[i++]));

		// other optical processes
		getMaterialPropertyFromString(pars[i++], "rayleigh");
	}
}


ostream& operator<<(ostream& stream, GMaterial gMat) {
	stream << endl;
	stream << "   - Material: " << gMat.name << "  in system  " << gMat.system << ": " << endl;
	stream << "     Density:          " << gMat.density << endl;
	if (!gMat.components.empty()) {
		stream << "     Composition:          " << endl;
		for (unsigned m = 0; m < gMat.components.size(); m++) {
			string quantity = gMat.amounts[m] > 1 ? " atoms " : " fractional mass";
			stream << "       ・ " << gMat.components[m] << quantity << " " << gMat.amounts[m] << endl;
		}
	}
	stream << "     Description: " << gMat.description << endl;
	stream << endl;

	return stream;
}


// sets components and amounts
void GMaterial::setComponentsFromString(const string& composition) {
	vector<string> allComponents = getStringVectorFromString(composition);

	for (unsigned e = 0; e < allComponents.size() / 2; e++) {
		components.push_back(allComponents[e * 2]);
		amounts.push_back(stod(allComponents[e * 2 + 1]));
	}
}


// load property from DB entry based on its name
void GMaterial::getMaterialPropertyFromString(const string& parameter, const string& propertyName) {
	// nothing to do if the parameter is not assigned
	if (removeLeadingAndTrailingSpacesFromString(parameter) == UNINITIALIZEDSTRINGQUANTITY) { return; }

	stringstream parameterComponents(parameter);

	while (!parameterComponents.eof()) {
		string component;
		parameterComponents >> component;

		// removing whitespaces
		string trimmedComponent = removeLeadingAndTrailingSpacesFromString(component);

		if (propertyName == "photonEnergy") { photonEnergy.push_back(getG4Number(trimmedComponent)); }
		else if (propertyName == "indexOfRefraction") { indexOfRefraction.push_back(getG4Number(trimmedComponent)); }
		else if (propertyName == "absorptionLength") { absorptionLength.push_back(getG4Number(trimmedComponent)); }
		else if (propertyName == "reflectivity") { reflectivity.push_back(getG4Number(trimmedComponent)); }
		else if (propertyName == "efficiency") { efficiency.push_back(getG4Number(trimmedComponent)); }
		else if (propertyName == "fastcomponent") { fastcomponent.push_back(getG4Number(trimmedComponent)); }
		else if (propertyName == "slowcomponent") { slowcomponent.push_back(getG4Number(trimmedComponent)); }
		else if (propertyName == "scintillationyield") { scintillationyield = getG4Number(trimmedComponent); }
		else if (propertyName == "resolutionscale") { resolutionscale = getG4Number(trimmedComponent); }
		else if (propertyName == "fasttimeconstant") { fasttimeconstant = getG4Number(trimmedComponent); }
		else if (propertyName == "slowtimeconstant") { slowtimeconstant = getG4Number(trimmedComponent); }
		else if (propertyName == "yieldratio") { yieldratio = getG4Number(trimmedComponent); }
		else if (propertyName == "birkConstant") { birkConstant = getG4Number(trimmedComponent); }
		else if (propertyName == "rayleigh") { rayleigh.push_back(getG4Number(trimmedComponent)); }


		if (propertyName == "rayleigh") {
			// rayleigh is the last quantity to be loaded
			// now we can check the vector sizes for comparison
			// if they do match, the behaviour is unknown, and we need to exit
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
