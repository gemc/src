/**
 * @mainpage GField Framework Documentation
 *
 * @section overview Overview
 * The GField framework provides an extensible mechanism for managing magnetic fields in GEMC simulations.
 * It allows users to define, instantiate, and manage various types of magnetic fields, including multipole fields.
 * The framework consists of several core classes:
 *
 * - **GField**: Abstract base class representing a magnetic field.
 * - **GFieldDefinition**: Utility struct for loading field configurations from goptions.
 * - **GMagneto**: Manages collections of fields and their corresponding Geant4 field managers.
 * - **GField_MultipolesFactory**: Implements a specific type of magnetic field characterized by multipoles.
 *
 * The framework supports dynamic loading of field configurations and seamless integration with Geant4 field management.
 *
 * @section conventions Conventions
 * - Fields are identified by unique string names.
 * - Integration steppers are selected based on their names; only supported steppers are allowed.
 * - Rotation axes for fields are encoded as integers:
 *   - `0` for X-axis
 *   - `1` for Y-axis
 *   - `2` for Z-axis
 * - Field strengths and other parameters are specified in standard Geant4 units.
 *
 * Error handling is consistent across all classes, ensuring that configuration errors are logged and handled gracefully.
 *
 * @section examples Examples
 *
 * ### Defining and Managing a Multipole Field:
 * ```cpp
 * // Initialize GOptions (parsed from YAML or another source)
 * GOptions* gopts = new GOptions("config.yaml");
 *
 * // Create a GMagneto instance to manage fields
 * GMagneto* magneto = new GMagneto(gopts);
 *
 * // Check if a specific field exists
 * if (magneto->isField("multipole_field")) {
 *     GField* field = magneto->getField("multipole_field");
 *     G4FieldManager* fieldManager = magneto->getFieldMgr("multipole_field");
 * }
 * ```
 *
 * ### Implementing a Custom Field:
 * ```cpp
 * class CustomField : public GField {
 * public:
 *     void GetFieldValue(const double pos[3], double* bfield) const override {
 *         // Implement custom field logic here
 *     }
 * };
 * ```
 *
 * @section api API Documentation
 *
 * ### Class GField
 * - **Description**: Abstract base class for magnetic fields.
 * - **Methods**:
 *   - `virtual void GetFieldValue(const double x[3], double* bfield) const = 0`: Computes the magnetic field vector at a given position.
 *   - `G4FieldManager* create_FieldManager()`: Creates a Geant4 field manager for the field.
 *
 * ### Class GFieldDefinition
 * - **Description**: Utility struct for field definitions.
 * - **Methods**:
 *   - `void init_parameters(string name, string stepper, double step, string type, int verbosity)`: Initializes field parameters.
 *   - `void add_map_parameter(string key, string value)`: Adds a key-value pair to the field's configuration.
 *   - `string gfieldPluginName()`: Returns the name of the field plugin.
 *
 * ### Class GMagneto
 * - **Description**: Manages multiple magnetic fields and their field managers.
 * - **Methods**:
 *   - `bool isField(string name)`: Checks if a field exists.
 *   - `GField* getField(string name)`: Retrieves a field by name.
 *   - `G4FieldManager* getFieldMgr(string name)`: Retrieves a field manager by name.
 *
 * ### Class GField_MultipolesFactory
 * - **Description**: Implements a multipole magnetic field.
 * - **Methods**:
 *   - `void GetFieldValue(const G4double pos[4], G4double* bfield) const`: Computes the multipole field vector at a given position.
 *
 */
