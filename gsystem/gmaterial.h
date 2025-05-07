#pragma once

// gemc
#include "glogger.h"

// c++
#include <string>
#include <vector>
#include <numeric>

using std::string;
using std::ostream;
using std::vector;


class GMaterial {
public:
	GMaterial(string system, vector<string> pars, std::shared_ptr<GLogger> logger);

	// Define a virtual clone method to be used in the copy constructor
	virtual std::unique_ptr<GMaterial> clone() const {
		return std::make_unique<GMaterial>(*this);  // Make a copy of the current object
	}

	// Virtual destructor, needed for when unique_ptr<GVolume> is deleted
	virtual ~GMaterial() = default;

private:
	string system;      // System of provenience
	string name;        // Name of the material
	string description; // Volume Description, for documentation

	// material properties
	// the material is recorded with a string 'components'
	// that contains both the element (material) name and its number of atoms (fractional mass)
	//
	// examples:
	// C 9 H 10
	// G4_N 0.7 G4_O 0.3
	//
	double         density;    // Material density, in g/cm3
	vector<string> components; // The list of atoms/material
	vector<double> amounts;    // Each component quantity or fractional mass

	// optical properties
	vector<double> photonEnergy;
	// A list of photon energies with units, at which any other optical parameters will be evaluated
	vector<double> indexOfRefraction; // A list of the refractive index evaluated at the energies named in photonEnergy
	vector<double> absorptionLength;
	// A list of the material absorption length with units, evaluated at the energies in photonEnergy
	vector<double> reflectivity; // A list of reflectivity values evaluated at the energies in photonEnergy
	vector<double> efficiency;   // A list of absorption efficiency evaluated at the energies in photonEnergy

	// scintillation properties
	vector<double> fastcomponent;
	// A list of the fast component relative spectra values evaluated at the energies in photonEnergy
	vector<double> slowcomponent;
	// A list of the fast component relative spectra values evaluated at the energies in photonEnergy
	double scintillationyield; // Characteristic light yield in photons/MeV e-, given as a single number
	double resolutionscale;    // Resolution scale broadens the statistical distribution of generated photons
	double fasttimeconstant;   // FIX ME believe this is related to the scintillator pulse rise time.
	double slowtimeconstant;   // FIX ME believe this is related to scintillator slow decay timefloat
	double yieldratio;
	// Relative strength of the fast component as a fraction of total scintillation yield, given as a single number
	double birkConstant; //  FIX ME

	// other optical properties
	vector<double> rayleigh;
	// A list of the Rayleigh scattering attenuation coefficient evaluated at the energies in photonEnergy

	// load material components from DB entry
	void setComponentsFromString(const string& composition);

	// load property from DB entry based on its name
	void getMaterialPropertyFromString(const string& parameter, const string& propertyName);

	friend ostream& operator<<(ostream& stream, GMaterial); // Logs infos on screen.

	std::shared_ptr<GLogger> log; ///< Logger instance for logging messages.

public:
	// getters
	[[nodiscard]] inline string getName() const { return name; }

	[[nodiscard]] inline string getDescription() const { return description; }

	[[nodiscard]] inline double getDensity() const { return density; }

	[[nodiscard]] inline vector<string> getComponents() const { return components; }

	[[nodiscard]] inline vector<double> getAmounts() const { return amounts; }

	// optical properties
	[[nodiscard]] inline vector<double> getPhotonEnergy() const { return photonEnergy; }

	[[nodiscard]] inline vector<double> getIndexOfRefraction() const { return indexOfRefraction; }

	[[nodiscard]] inline vector<double> getAbsorptionLength() const { return absorptionLength; }

	[[nodiscard]] inline vector<double> getReflectivity() const { return reflectivity; }

	[[nodiscard]] inline vector<double> getEfficiency() const { return efficiency; }

	// scintillation properties
	[[nodiscard]] inline vector<double> getFastcomponent() const { return fastcomponent; }

	[[nodiscard]] inline vector<double> getSlowcomponent() const { return slowcomponent; }

	[[nodiscard]] inline double getScintillationyield() const { return scintillationyield; }

	[[nodiscard]] inline double getResolutionscale() const { return resolutionscale; }

	[[nodiscard]] inline double getFasttimeconstant() const { return fasttimeconstant; }

	[[nodiscard]] inline double getSlowtimeconstant() const { return slowtimeconstant; }

	[[nodiscard]] inline double getYieldratio() const { return yieldratio; }

	[[nodiscard]] inline double getBirkConstant() const { return birkConstant; }

	// other optical properties
	[[nodiscard]] inline vector<double> getRayleigh() const { return rayleigh; }

	// return true if this is a chemical formula (the sum of all the component amounts is > 1)
	[[nodiscard]] inline bool isChemicalFormula() const {
		return std::accumulate(amounts.begin(), amounts.end(), 0.0) > 1;
	}

};

