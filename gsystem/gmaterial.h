#pragma once

// gemc
#include "gbase.h"

// c++
#include <string>
#include <vector>
#include <numeric>

/**
 * \ingroup gemc_gsystem_core
 *
 * \class GMaterial
 * \brief Material definition belonging to a detector system.
 *
 * A GMaterial is constructed from a serialized list of parameters (typically read from a
 * database row or ASCII file) and stores:
 * - bulk density;
 * - composition (components + amounts);
 * - optional optical properties (refractive index, absorption length, etc.);
 * - optional scintillation properties (yield, time constants, spectra, etc.).
 *
 * The module treats materials as system-local definitions: a system (GSystem) loads
 * its own materials and associates them to volumes by name.
 *
 */
class GMaterial : public GBase<GMaterial>
{
public:
	/**
	 * \brief Construct a material from a serialized parameter list.
	 *
	 * \param system System name that owns this material (used for provenance/logging).
	 * \param pars   Serialized material parameters (must match GMATERIALNUMBEROFPARS).
	 * \param logger Logger used for diagnostics and error reporting.
	 *
	 * \details
	 * The constructor validates \c pars length against GMATERIALNUMBEROFPARS and then
	 * parses the fields. Optical/scintillation properties may be "unset" (typically
	 * UNINITIALIZEDSTRINGQUANTITY) and are skipped.
	 */
	GMaterial(const std::string& system, std::vector<std::string> pars, const std::shared_ptr<GLogger>& logger);

	/**
	 * \brief Clone the material (polymorphic deep-copy).
	 *
	 * \return A heap-allocated copy of this material.
	 */
	[[nodiscard]] virtual std::unique_ptr<GMaterial> clone() const {
		return std::make_unique<GMaterial>(*this);
	}

	/// \brief Virtual destructor (safe deletion through base pointers).
	virtual ~GMaterial() = default;

private:
	std::string system;      ///< System of provenance (which detector subsystem defines this material).
	std::string name;        ///< Material name (key used for volume→material association).
	std::string description; ///< Human-readable description for documentation and logs.

	/**
	 * \name Bulk and composition
	 * \brief Density and component makeup of the material.
	 *
	 * The material composition is stored as parallel vectors:
	 * - \c components contains element/material identifiers;
	 * - \c amounts contains either atom counts or fractional masses, depending on convention.
	 *
	 * Examples of the serialized composition field:
	 * - \c "C 9 H 10"
	 * - \c "G4_N 0.7 G4_O 0.3"
	 */
	///@{
	double                   density{};    ///< Material density, in g/cm3.
	std::vector<std::string> components;   ///< Component identifiers (elements or referenced materials).
	std::vector<double>      amounts;      ///< Component amounts: integer-ish atoms or fractional mass.
	///@}

	/**
	 * \name Optical properties
	 * \brief Tabulated optical properties evaluated over \c photonEnergy.
	 *
	 * Vector properties are expected to match the length of \c photonEnergy when provided.
	 * Consistency is validated during parsing once the final optical vector is loaded.
	 */
	///@{
	std::vector<double> photonEnergy;       ///< Photon energies (with units) at which properties are tabulated.
	std::vector<double> indexOfRefraction;  ///< Refractive index values evaluated at \c photonEnergy.
	std::vector<double> absorptionLength;   ///< Absorption length values evaluated at \c photonEnergy.
	std::vector<double> reflectivity;       ///< Reflectivity values evaluated at \c photonEnergy.
	std::vector<double> efficiency;         ///< Detection/absorption efficiency evaluated at \c photonEnergy.
	///@}

	/**
	 * \name Scintillation properties
	 * \brief Scintillation spectra and associated scalar parameters.
	 *
	 * Spectra are stored as vectors evaluated at \c photonEnergy, while yields and constants
	 * are stored as scalars.
	 */
	///@{
	std::vector<double> fastcomponent;      ///< Fast scintillation spectrum values evaluated at \c photonEnergy.
	std::vector<double> slowcomponent;      ///< Slow scintillation spectrum values evaluated at \c photonEnergy.

	double scintillationyield{};            ///< Light yield in photons/MeV (single value).
	double resolutionscale{};               ///< Broadens the photon statistics distribution.
	double fasttimeconstant{};              ///< Fast scintillation time constant (time units).
	double slowtimeconstant{};              ///< Slow scintillation time constant (time units).
	double yieldratio{};                    ///< Fraction of total yield attributed to the fast component.
	double birksConstant{};                 ///< Birks constant for quenching model (units depend on convention).
	///@}

	/**
	 * \name Additional optical properties
	 * \brief Other tabulated optical processes evaluated over \c photonEnergy.
	 */
	///@{
	std::vector<double> rayleigh;           ///< Rayleigh scattering attenuation coefficients evaluated at \c photonEnergy.
	///@}

	/**
	 * \brief Parse a "components + amounts" string into vectors.
	 *
	 * \param composition Tokenized string containing alternating (component, amount) pairs.
	 *
	 * \details The string is split into tokens and interpreted as:
	 * \code
	 * components[0] amounts[0] components[1] amounts[1] ...
	 * \endcode
	 *
	 * If the token count is odd, the trailing token is ignored by construction.
	 */
	void setComponentsFromString(const std::string& composition);

	/**
	 * \brief Parse and store one property vector (or scalar) based on property name.
	 *
	 * \param parameter Raw parameter string read from configuration (may be unset).
	 * \param propertyName Name identifying which property is being loaded.
	 *
	 * \details This method tokenizes the input string and converts each token to a numeric value.
	 * For vector properties, values are appended; for scalar properties, the last parsed value wins.
	 *
	 * The method performs a vector-size consistency check once the final optical vector is processed,
	 * ensuring all provided property vectors match \c photonEnergy.
	 */
	void getMaterialPropertyFromString(const std::string& parameter, const std::string& propertyName);

	/// \brief Stream operator used for logging material summaries.
	friend std::ostream& operator<<(std::ostream& stream, const GMaterial&);

	/**
	 * \brief Assign a scalar parameter if it is set and parseable.
	 *
	 * \param pars Vector of raw parameters.
	 * \param i    Current parsing index (incremented on use).
	 * \param out  Output scalar to assign when set.
	 * \return \c true if a value was assigned, \c false otherwise.
	 *
	 * \details This helper interprets "unset" values using the same convention as parsing
	 * utilities (e.g. UNINITIALIZEDSTRINGQUANTITY). Malformed scalars are treated as unset.
	 */
	bool assign_if_set(const std::vector<std::string>& pars, size_t& i, double& out);

public:
	/// \name Identity and description
	///@{
	[[nodiscard]] std::string              getName() const { return name; }
	[[nodiscard]] std::string              getDescription() const { return description; }
	[[nodiscard]] double                   getDensity() const { return density; }
	[[nodiscard]] std::vector<std::string> getComponents() const { return components; }
	[[nodiscard]] std::vector<double>      getAmounts() const { return amounts; }
	///@}

	/// \name Optical properties
	///@{
	[[nodiscard]] std::vector<double> getPhotonEnergy() const { return photonEnergy; }
	[[nodiscard]] std::vector<double> getIndexOfRefraction() const { return indexOfRefraction; }
	[[nodiscard]] std::vector<double> getAbsorptionLength() const { return absorptionLength; }
	[[nodiscard]] std::vector<double> getReflectivity() const { return reflectivity; }
	[[nodiscard]] std::vector<double> getEfficiency() const { return efficiency; }
	///@}

	/// \name Scintillation properties
	///@{
	[[nodiscard]] std::vector<double> getFastcomponent() const { return fastcomponent; }
	[[nodiscard]] std::vector<double> getSlowcomponent() const { return slowcomponent; }
	[[nodiscard]] double              getScintillationyield() const { return scintillationyield; }
	[[nodiscard]] double              getResolutionscale() const { return resolutionscale; }
	[[nodiscard]] double              getFasttimeconstant() const { return fasttimeconstant; }
	[[nodiscard]] double              getSlowtimeconstant() const { return slowtimeconstant; }
	[[nodiscard]] double              getYieldratio() const { return yieldratio; }
	[[nodiscard]] double              getBirksConstant() const { return birksConstant; }
	///@}

	/// \name Additional optical properties
	///@{
	[[nodiscard]] std::vector<double> getRayleigh() const { return rayleigh; }
	///@}

	/**
	 * \brief Heuristic: return true if the composition looks like a chemical formula.
	 *
	 * The module uses a simple heuristic: when the sum of \c amounts is > 1.0, the
	 * values are likely atom counts rather than fractional masses.
	 *
	 * \return \c true if the composition likely represents a formula, \c false otherwise.
	 */
	[[nodiscard]] bool isChemicalFormula() const {
		return std::accumulate(amounts.begin(), amounts.end(), 0.0) > 1;
	}
};
