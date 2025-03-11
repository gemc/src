#ifndef  GFIELDS_H
#define  GFIELDS_H  1

// G4 headers
#include "G4MagneticField.hh"
#include "G4FieldManager.hh"
#include "G4MagIntegratorStepper.hh"

// gemc
#include "gfactory.h"
#include "goptions.h"
#include "gfieldConventions.h"

/**
 * @brief Utility struct to load GFields from options.
 */
struct GFieldDefinition {
    /**
     * @brief Default constructor.
     */
    GFieldDefinition() {}

    std::string name; ///< Key in the gmagneto maps.
    std::string integration_stepper; ///< Type of integration stepper.
    double minimum_step; ///< Minimum step size for integration.
    std::string type; ///< Type of the field.

    map <std::string, string> field_parameters; ///< Field parameters as key-value pairs.

    /**
     * @brief Adds a parameter to the field parameters map.
     * @param key Parameter key.
     * @param value Parameter value.
     */
    void add_map_parameter(std::string key, std::string value) {
        field_parameters[key] = value;
    }

    /**
     * @brief Gets the plugin name for the field.
     * @return Plugin name as a string.
     */
    string gfieldPluginName() {
        return "gfield" + type + "Factory";
    }

    // overload << to print the field definition
    friend std::ostream &operator<<(std::ostream &stream, GFieldDefinition gfd) {
        stream << "  > Field name:            " << gfd.name << std::endl;
        stream << "    - integration stepper  " << gfd.integration_stepper << std::endl;
        stream << "    - minimum step         " << gfd.minimum_step << " mm" << std::endl;
        stream << "    - type                 " << gfd.type << std::endl;

        // print the field parameters
        // align the keys to the left
        for (auto &field_parameter: gfd.field_parameters) {
            stream << "    - " << std::left << std::setw(21) << field_parameter.first  << field_parameter.second << std::endl;
        }

        return stream;
    }
};

/**
 * @brief Abstract base class representing a magnetic field.
 */
class GField : public G4MagneticField {

public:
    /**
        * @brief Default constructor.
        */
    GField() = default;

    /**
     * @brief Virtual destructor.
     */
    virtual ~GField() = default;

    /**
     * @brief Pure virtual function to get the magnetic field value.
     * @param x Position in space.
     * @param bfield Magnetic field vector.
     */
    virtual void GetFieldValue(const double x[3], double *bfield) const = 0;

    /**
     * @brief Creates the G4FieldManager for the field.
     * @return Pointer to the G4FieldManager.
     */
    G4FieldManager *create_FieldManager();

    /**
     * @brief Sets the field definition for the field.
     * @param gfd Field definition to set.
     */
    virtual void load_field_definitions(GFieldDefinition gfd) {
        gfield_definitions = gfd;
    }

    /**
      * @brief Logs a message with the field context.
      * @param message Message to log.
     */
    void gFLogMessage(std::string message) {
        gLogMessage(GFIELDLOGHEADER + gfield_definitions.name + " " + message);
    }

    int get_field_parameter_int(std::string key) {
        return stoi(gfield_definitions.field_parameters[key]);
    }
    double get_field_parameter_double(std::string key) {
        return stod(gfield_definitions.field_parameters[key]);
    }

private:


    std::vector <std::string> SUPPORTED_STEPPERS = { ///< Supported integration steppers.
            "G4DormandPrince745", "G4ClassicalRK4", "G4SimpleRunge", "G4HelixExplicitEuler",
            "G4HelixImplicitEuler", "G4CashKarpRKF45", "G4RKG3_Stepper", "G4SimpleHeum",
            "G4NystromRK4", "G4ImplicitEuler", "G4ExplicitEuler"
    };


protected:
    GFieldDefinition gfield_definitions;

public:

    static GField *instantiate(const dlhandle handle) {

        if (handle == nullptr) return nullptr;

        // must match the extern C declaration in the derived factories
        void *maker = dlsym(handle, "GFieldFactory");

        if (maker == nullptr) return nullptr;

        typedef GField *(*fptr)();

        // static_cast not allowed here
        // see http://stackoverflow.com/questions/573294/when-to-use-reinterpret-cast
        // need to run the DLL GMediaFactory function that returns the factory
        fptr func = reinterpret_cast<fptr>(reinterpret_cast<void *>(maker));

        return func();
    }
};


#endif
