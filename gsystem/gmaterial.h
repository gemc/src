#pragma once

// gemc
#include "glogger.h"

// c++
#include <string>
#include <vector>
#include <numeric>

class GMaterial {
public:
	GMaterial(std::string system, std::vector<std::string> pars, std::shared_ptr<GLogger> logger);

	// Define a virtual clone method to be used in the copy constructor
	[[nodiscard]] virtual std::unique_ptr<GMaterial> clone() const {
		return std::make_unique<GMaterial>(*this); // Make a copy of the current object
	}

	// Virtual destructor, needed for when unique_ptr<GVolume> is deleted
	virtual ~GMaterial() = default;

private:
	std::string system;      // System of provenience
	std::string name;        // Name of the material
	std::string description; // Volume Description, for documentation

	// material properties
	// the material is recorded with the string 'components'
	// that contains both the element (material) name and its number of atoms (fractional mass)
	//
	// examples:
	// C 9 H 10
	// G4_N 0.7 G4_O 0.3
	//
	double                   density;    // Material density, in g/cm3
	std::vector<std::string> components; // The list of atoms/material
	std::vector<double>      amounts;    // Each component quantity or fractional mass

	// optical properties
	std::vector<double> photonEnergy;
	// A list of photon energies with units, at which any other optical parameters will be evaluated
	std::vector<double> indexOfRefraction; // A list of the refractive index evaluated at the energies named in photonEnergy
	std::vector<double> absorptionLength;
	// A list of the material absorption length with units, evaluated at the energies in photonEnergy
	std::vector<double> reflectivity; // A list of reflectivity values evaluated at the energies in photonEnergy
	std::vector<double> efficiency;   // A list of absorption efficiency evaluated at the energies in photonEnergy

	// scintillation properties
	std::vector<double> fastcomponent;
	// A list of the fast component relative spectra values evaluated at the energies in photonEnergy
	std::vector<double> slowcomponent;
	// A list of the fast component relative spectra values evaluated at the energies in photonEnergy
	double scintillationyield; // Characteristic light yield in photons/MeV e-, given as a single number
	double resolutionscale;    // Resolution scale broadens the statistical distribution of generated photons
	double fasttimeconstant;   // FIX ME believe this is related to the scintillator pulse rise time
	double slowtimeconstant;   // FIX ME believe this is related to scintillator slow decay time
	double yieldratio;
	// Relative strength of the fast component as a fraction of total scintillation yield, given as a single number
	double birkConstant; //  FIX ME

	// other optical properties
	std::vector<double> rayleigh;
	// A list of the Rayleigh scattering attenuation coefficient evaluated at the energies in photonEnergy

	// load material components from DB entry
	void setComponentsFromString(const std::string& composition);

	// load property from DB entry based on its name
	void getMaterialPropertyFromString(const std::string& parameter, const std::string& propertyName);

	friend std::ostream& operator<<(std::ostream& stream, GMaterial); // Logs infos on screen.

	std::shared_ptr<GLogger> log; ///< Logger instance for logging messages.

public:
	// getters
	[[nodiscard]] std::string getName() const { return name; }

	[[nodiscard]] std::string getDescription() const { return description; }

	[[nodiscard]] double getDensity() const { return density; }

	[[nodiscard]] std::vector<std::string> getComponents() const { return components; }

	[[nodiscard]] std::vector<double> getAmounts() const { return amounts; }

	// optical properties
	[[nodiscard]] std::vector<double> getPhotonEnergy() const { return photonEnergy; }

	[[nodiscard]] std::vector<double> getIndexOfRefraction() const { return indexOfRefraction; }

	[[nodiscard]] std::vector<double> getAbsorptionLength() const { return absorptionLength; }

	[[nodiscard]] std::vector<double> getReflectivity() const { return reflectivity; }

	[[nodiscard]] std::vector<double> getEfficiency() const { return efficiency; }

	// scintillation properties
	[[nodiscard]] std::vector<double> getFastcomponent() const { return fastcomponent; }

	[[nodiscard]] std::vector<double> getSlowcomponent() const { return slowcomponent; }

	[[nodiscard]] double getScintillationyield() const { return scintillationyield; }

	[[nodiscard]] double getResolutionscale() const { return resolutionscale; }

	[[nodiscard]] double getFasttimeconstant() const { return fasttimeconstant; }

	[[nodiscard]] double getSlowtimeconstant() const { return slowtimeconstant; }

	[[nodiscard]] double getYieldratio() const { return yieldratio; }

	[[nodiscard]] double getBirkConstant() const { return birkConstant; }

	// other optical properties
	[[nodiscard]] std::vector<double> getRayleigh() const { return rayleigh; }

	// return true if this is a chemical formula (the sum of all the component amounts is > 1)
	[[nodiscard]] bool isChemicalFormula() const { return std::accumulate(amounts.begin(), amounts.end(), 0.0) > 1; }

};
