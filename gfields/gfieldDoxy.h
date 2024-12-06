/**
 * @mainpage GField Framework Documentation
 *
 * @section overview Overview
 * The GField framework provides an extensible mechanism for managing magnetic fields in GEMC simulations.
 * It allows users to define, instantiate, and manage various types of magnetic fields, including multipoles fields.
 * The users definitions are external to the framework and are loaded dynamically at runtime through plugins.
 * The framework consists of several core classes:
 *
 * - **GField**: Abstract base class representing a magnetic field.
 * - **GFieldDefinition**: Utility struct for loading field configurations from goptions. New variable can be added
 *  using the `add_map_parameter` method.
 * - **GMagneto**: Manages collections of fields and their corresponding Geant4 field managers.
 *
 * @section factories Field factories implemented within GEMC:
 * - **GField_MultipolesFactory**: Implements a specific type of magnetic field characterized by multipoles.
 *
 * The framework supports dynamic loading of field definitions and seamless integration with Geant4 field management.
 *
 * @section parameters Field Parameters
 * - Fields are identified by unique string names.
 * - A volume can be associated to a magnetic field by using the field name.
 * - Integration steppers are selected based on their names; only supported steppers are allowed. The default is `G4DormandPrince745`.
 * - Rotation axes for fields are encoded as integers:
 *   - `0` for X-axis
 *   - `1` for Y-axis
 *   - `2` for Z-axis
 * - Field strengths and other parameters are specified in standard Geant4 units.
 *
 * TODO
 *
 * ### Implementing a Custom Field:
 * To be completed
 *
 * @section api API Documentation
 *
 * ### Class GField
 * - **Description**: Abstract base class for magnetic fields.
 * - **Relevant Methods**:
 *   - `virtual void GetFieldValue(const double x[3], double* bfield)`: Computes the magnetic field vector at a given position.
 *   - `G4FieldManager* create_FieldManager()`: Creates a Geant4 field manager for the field. The field manager is created
 *   automatically by gemc using `G4Mag_UsualEqRhs` and the basic field parameters:
 *   - integration_stepper: The integration stepper to use.
 *   - minimum_step: The minimum step for the G4ChordFinder.
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
 *
 *
 * \subsection cisubsection Continuous Integration
 * The GFields framework is continuously integrated and tested to ensure stability and reliability across updates.
 *
 * \n\n
 * \author \n &copy; Maurizio Ungaro
 * \author e-mail: ungaro@jlab.org
 * \n\n\n
 */
