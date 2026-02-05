#pragma once

// G4 headers
#include "G4MagneticField.hh"
#include "G4FieldManager.hh"

// gemc
#include "gfactory.h"
#include "gbase.h"


constexpr const char* GFIELD_LOGGER   = "gfield";
constexpr const char* GMAGNETO_LOGGER = "gmagneto";

/**
 * @brief Lightweight configuration carrier used to load and configure a \ref GField "GField" plugin.
 * @ingroup gfield_module
 *
 * A \ref GFieldDefinition "GFieldDefinition" is typically built from user options (see gfields::get_GFieldDefinition()).
 * It is then passed to a concrete field via \ref GField::load_field_definitions "load_field_definitions()".
 *
 * Notes on plugin naming:
 * - The plugin shared-library entry point is expected to expose the C symbol \c GFieldFactory.
 * - The library name or plugin identifier is derived from \ref GFieldDefinition::gfieldPluginName "gfieldPluginName()".
 */
struct GFieldDefinition {
	/**
	 * @brief Default constructor.
	 */
	GFieldDefinition() = default;

	/// Field name key used by \ref GMagneto "GMagneto" maps.
	std::string name;

	/// Integration stepper name (string) used when creating the \c G4ChordFinder.
	std::string integration_stepper;

	/// Minimum step size used when constructing the \c G4ChordFinder (Geant4 length units).
	double minimum_step = 0.0;

	/// Field type discriminator used to derive the plugin factory name (e.g. "multipoles").
	std::string type;

	/// Field parameters stored as string expressions (often including unit expressions).
	std::map<std::string, std::string> field_parameters;

	/**
	 * @brief Add or overwrite a parameter in the field-parameter map.
	 * @param key Parameter key (e.g. "strength").
	 * @param value Parameter value expression (e.g. "2.0*tesla").
	 */
	void add_map_parameter(const std::string& key, const std::string& value) { field_parameters[key] = value; }

	/**
	 * @brief Derive the plugin name for the field definition.
	 * @return Plugin name string.
	 *
	 * Convention:
	 * - This method returns \c "gfield" + type + \c "Factory".
	 * - For example, a type of \c "multipoles" yields \c "gfieldmultipolesFactory".
	 */
	std::string gfieldPluginName() const { return "gfield" + type + "Factory"; }

	/**
	 * @brief Stream formatter used for logging/debug output.
	 * @param stream Output stream.
	 * @param gfd Field definition to print.
	 * @return The same stream for chaining.
	 *
	 * This prints the core integration parameters and then each entry in \ref GFieldDefinition::field_parameters "field_parameters".
	 */
	friend std::ostream& operator<<(std::ostream& stream, GFieldDefinition gfd) {
		stream << "  > Field name:            " << gfd.name << std::endl;
		stream << "    - integration stepper  " << gfd.integration_stepper << std::endl;
		stream << "    - minimum step         " << gfd.minimum_step << " mm" << std::endl;
		stream << "    - type                 " << gfd.type << std::endl;

		// Print the field parameters, aligning keys for readability.
		for (auto& field_parameter : gfd.field_parameters) {
			stream << "    - " << std::left << std::setw(21) << field_parameter.first << field_parameter.second << std::endl;
		}

		return stream;
	}
};

/**
 * @brief Abstract base class representing a magnetic field.
 * @ingroup gfield_module
 *
 * A concrete field implementation must provide \ref GField::GetFieldValue "GetFieldValue()" and may override
 * \ref GField::load_field_definitions "load_field_definitions()" to parse/cache configuration from a \ref GFieldDefinition "GFieldDefinition".
 *
 * Integration:
 * - The field is also a \c G4MagneticField and is used by Geant4 stepping infrastructure.
 * - The helper \ref GField::create_FieldManager "create_FieldManager()" constructs a \c G4FieldManager and its
 *   chord finder using a supported stepper name.
 */
class GField : public GBase<GField>, public G4MagneticField {

public:
	/**
	 * @brief Construct the field base with the provided options (for logging and configuration access).
	 * @param gopt Shared options object.
	 */
	explicit GField(const std::shared_ptr<GOptions>& gopt) : GBase(gopt, GFIELD_LOGGER) {}

	/**
	 * @brief Compute the magnetic field vector at a given position.
	 * @param x Position in the lab frame as \c {x,y,z}.
	 * @param bfield Output array filled with \c {Bx,By,Bz}.
	 *
	 * This method is the key interface used by Geant4 when transporting tracks in a field.
	 */
	virtual void GetFieldValue(const double x[3], double* bfield) const = 0;

	/**
	 * @brief Create a \c G4FieldManager configured for this field.
	 * @return Newly allocated \c G4FieldManager pointer.
	 *
	 * The manager is created using:
	 * - \c G4Mag_UsualEqRhs as the equation of motion,
	 * - one of the supported integration steppers (selected by name),
	 * - a \c G4ChordFinder constructed with the configured minimum step.
	 *
	 * Ownership:
	 * - The returned object is heap-allocated. In this module it is owned by \ref GMagneto "GMagneto".
	 */
	G4FieldManager* create_FieldManager();

	/**
	 * @brief Store the field definition used to configure this field instance.
	 * @param gfd Field definition to copy into internal storage.
	 *
	 * Concrete implementations typically override this method to parse and cache frequently used parameters.
	 */
	virtual void load_field_definitions(GFieldDefinition gfd) { gfield_definitions = gfd; }

	/**
	 * @brief Convenience accessor for integer-valued parameters stored in \ref GFieldDefinition::field_parameters "field_parameters".
	 * @param key Map key to retrieve.
	 * @return Parsed integer value.
	 */
	int get_field_parameter_int(const std::string& key) { return stoi(gfield_definitions.field_parameters[key]); }

	/**
	 * @brief Convenience accessor for floating-point parameters stored in \ref GFieldDefinition::field_parameters "field_parameters".
	 * @param key Map key to retrieve.
	 * @return Parsed floating-point value.
	 */
	double get_field_parameter_double(const std::string& key) { return stod(gfield_definitions.field_parameters[key]); }

	/**
	 * @brief Hook for configuring module loggers from options.
	 * @param g Shared options object.
	 *
	 * This module currently does not add additional logger wiring here.
	 */
	void set_loggers([[ maybe_unused ]] const std::shared_ptr<GOptions>& g) {
	}

private:
	/**
	 * @brief Supported Geant4 integrator stepper names.
	 *
	 * The names in this list are compared against the user-provided \ref GFieldDefinition::integration_stepper "integration_stepper".
	 * If the requested stepper is not found, the default from gfieldConventions.h is used.
	 */
	std::vector<std::string> SUPPORTED_STEPPERS = {
		"G4DormandPrince745",
		"G4ClassicalRK4",
		"G4SimpleRunge",
		"G4HelixExplicitEuler",
		"G4HelixImplicitEuler",
		"G4CashKarpRKF45",
		"G4RKG3_Stepper",
		"G4SimpleHeum",
		"G4NystromRK4",
		"G4ImplicitEuler",
		"G4ExplicitEuler"
	};

protected:
	/// Stored field definition used for configuration and logging.
	GFieldDefinition gfield_definitions;

public:
	/**
	 * @brief Instantiate a field object from a plugin handle.
	 * @param h Dynamic library handle.
	 * @param g Shared options passed to the plugin factory.
	 * @return Newly allocated field pointer, or \c nullptr on failure.
	 *
	 * The plugin must export a C symbol named \c "GFieldFactory" with a compatible signature.
	 * This method looks up that symbol and calls it to construct the concrete \ref GField "GField" instance.
	 */
	static GField* instantiate(const dlhandle h, const std::shared_ptr<GOptions> g) {
		if (!h) return nullptr;
		using fptr = GField* (*)(const std::shared_ptr<GOptions>&);

		// Must match the extern "C" declaration in the derived factories.
		auto sym = dlsym(h, "GFieldFactory");
		if (!sym) return nullptr;

		auto func = reinterpret_cast<fptr>(sym);
		return func(g);
	}
};
