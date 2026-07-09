#pragma once

// gemc
#include <gemc/gbase/gbase.h>

// c++
#include <string>
#include <vector>

/**
 * \ingroup gemc_gsystem_core
 *
 * \class GMirror
 * \brief Optical boundary (surface) definition belonging to a detector system.
 *
 * In gemc, any optical boundary is described as a "mirror", regardless of its use or
 * reflective quality. A GMirror is constructed from a serialized list of parameters
 * (typically read from a database row or ASCII file) and stores:
 * - the Geant4 surface configuration (type, finish, model, sigmaAlpha);
 * - the boundary scope: \c "SkinSurface" for the whole volume skin, or the name of a
 *   bordering volume for a border surface;
 * - the boundary optical properties, either as the name of a material whose properties
 *   table is reused (\c matOptProps) or as explicit tables evaluated at \c photonEnergy.
 *
 * The module treats mirrors as system-local definitions: a system (GSystem) loads its
 * own mirrors and volumes reference them by name through the GVolume \c mirror field.
 */
class GMirror : public GBase<GMirror> {
public:
	/**
	 * \brief Construct a mirror from a serialized parameter list.
	 *
	 * \param system System name that owns this mirror (used for provenance/logging).
	 * \param pars   Serialized mirror parameters (must match GMIRRORNUMBEROFPARS).
	 * \param logger Logger used for diagnostics and error reporting.
	 *
	 * \details
	 * The constructor validates \c pars length against GMIRRORNUMBEROFPARS and then parses
	 * the fields positionally, in the canonical order used by the loaders:
	 * name, description, type, finish, model, border, matOptProps, photonEnergy,
	 * indexOfRefraction, reflectivity, efficiency, specularlobe, specularspike,
	 * backscatter, transmittance, sigmaAlpha.
	 *
	 * Validation performed here (fail fast, before any Geant4 object is created):
	 * - type, finish, model and border are mandatory;
	 * - either \c matOptProps or a non-empty \c photonEnergy table must be provided;
	 * - every provided property vector must match the \c photonEnergy length.
	 */
	GMirror(const std::string& system, std::vector<std::string> pars, const std::shared_ptr<GLogger>& logger);

	/**
	 * \brief Clone the mirror (polymorphic deep-copy).
	 *
	 * \return A heap-allocated copy of this mirror.
	 */
	[[nodiscard]] virtual std::unique_ptr<GMirror> clone() const { return std::make_unique<GMirror>(*this); }

	/// \brief Virtual destructor (safe deletion through base pointers).
	virtual ~GMirror() = default;

private:
	std::string system;      ///< System of provenance (which detector subsystem defines this mirror).
	std::string name;        ///< Mirror name (key used for volume→mirror association).
	std::string description; ///< Human-readable description for documentation and logs.

	/**
	 * \name Geant4 surface configuration
	 * \brief Strings mapped to the G4OpticalSurface type/finish/model enums downstream.
	 */
	///@{
	std::string type;   ///< Surface type, e.g. \c "dielectric_metal".
	std::string finish; ///< Surface finish, e.g. \c "polished".
	std::string model;  ///< Optical model, e.g. \c "unified".
	///@}

	/**
	 * \brief Boundary scope.
	 *
	 * \c GMIRRORSKINSURFACE when the optical boundary represents the entire outside
	 * surface of the volume; otherwise the name of the bordering volume (same system)
	 * used to build a border surface.
	 */
	std::string border;

	/**
	 * \brief Name of a material whose properties table provides the boundary properties.
	 *
	 * Empty when unset: in that case the explicit tables below are used instead.
	 */
	std::string matOptProps;

	/**
	 * \name Optical properties
	 * \brief Tabulated boundary properties evaluated over \c photonEnergy.
	 *
	 * Vector properties must match the length of \c photonEnergy when provided.
	 * Consistency is validated at construction.
	 */
	///@{
	std::vector<double> photonEnergy;      ///< Photon energies (with units) at which properties are tabulated.
	std::vector<double> indexOfRefraction; ///< Refractive index values evaluated at \c photonEnergy.
	std::vector<double> reflectivity;      ///< Reflectivity values evaluated at \c photonEnergy.
	std::vector<double> efficiency;        ///< Photoelectric absorption efficiency evaluated at \c photonEnergy.
	std::vector<double> specularlobe;      ///< Specular lobe constants (rough surface scattering).
	std::vector<double> specularspike;     ///< Specular spike constants (rough surface scattering).
	std::vector<double> backscatter;       ///< Backscatter constants (rough surface scattering).
	std::vector<double> transmittance;     ///< Transmission probability (semi-transparent mirrors).
	///@}

	double sigmaAlpha{};     ///< Surface roughness parameter (unified model).
	bool   sigmaAlphaSet{};  ///< True when sigmaAlpha was explicitly provided.

	/**
	 * \brief Parse and store one property vector based on property name.
	 *
	 * \param parameter Raw parameter string read from configuration (may be unset).
	 * \param propertyName Name identifying which property is being loaded.
	 *
	 * \details Unset parameters (empty, "NULL", "none", "notDefined") are skipped.
	 * Each token is converted with unit support (e.g. \c "2.034*eV").
	 */
	void getMirrorPropertyFromString(const std::string& parameter, const std::string& propertyName);

	/// \brief Stream operator used for logging mirror summaries.
	friend std::ostream& operator<<(std::ostream& stream, const GMirror&);

public:
	/// \name Identity and description
	///@{
	[[nodiscard]] std::string getName() const { return name; }
	[[nodiscard]] std::string getDescription() const { return description; }
	///@}

	/// \name Geant4 surface configuration
	///@{
	[[nodiscard]] std::string getType() const { return type; }
	[[nodiscard]] std::string getFinish() const { return finish; }
	[[nodiscard]] std::string getModel() const { return model; }
	[[nodiscard]] std::string getBorder() const { return border; }
	[[nodiscard]] bool        isSkinSurface() const;
	[[nodiscard]] double      getSigmaAlpha() const { return sigmaAlpha; }
	[[nodiscard]] bool        hasSigmaAlpha() const { return sigmaAlphaSet; }
	///@}

	/// \name Optical properties
	///@{
	[[nodiscard]] std::string getMatOptProps() const { return matOptProps; }
	[[nodiscard]] bool        usesMaterialOpticalProperties() const { return !matOptProps.empty(); }
	[[nodiscard]] std::vector<double> getPhotonEnergy() const { return photonEnergy; }
	[[nodiscard]] std::vector<double> getIndexOfRefraction() const { return indexOfRefraction; }
	[[nodiscard]] std::vector<double> getReflectivity() const { return reflectivity; }
	[[nodiscard]] std::vector<double> getEfficiency() const { return efficiency; }
	[[nodiscard]] std::vector<double> getSpecularLobe() const { return specularlobe; }
	[[nodiscard]] std::vector<double> getSpecularSpike() const { return specularspike; }
	[[nodiscard]] std::vector<double> getBackscatter() const { return backscatter; }
	[[nodiscard]] std::vector<double> getTransmittance() const { return transmittance; }
	///@}
};
