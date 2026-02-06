/**
 * @mainpage GField framework
 *
 * @section gfield_intro Introduction
 * The GField framework is a plugin-driven module that provides magnetic-field support for GEMC simulations.
 * Users configure one or more magnetic fields through options (typically YAML-derived), and the framework:
 * - translates those options into \ref GFieldDefinition "GFieldDefinition" objects,
 * - dynamically loads field plugins,
 * - instantiates concrete \ref GField "GField" implementations,
 * - and creates a \c G4FieldManager for each field so Geant4 can integrate particle motion in the field.
 *
 * The core API is intentionally small:
 * - \ref GField "GField" defines the field interface (evaluation + manager construction support).
 * - \ref GFieldDefinition "GFieldDefinition" carries configuration and plugin identity.
 * - \ref GMagneto "GMagneto" is the owner/registry that loads fields and holds their managers.
 *
 * @section gfield_ownership Ownership and lifecycle
 * - **Configuration ownership:** options are owned externally (e.g. by the application); the module receives them as shared pointers.
 * - **Field ownership:** \ref GMagneto "GMagneto" owns all instantiated field objects and keeps them alive for its lifetime.
 * - **Manager ownership:** \ref GMagneto "GMagneto" also owns the \c G4FieldManager objects returned by
 *   \ref GField::create_FieldManager "create_FieldManager()".
 *
 * In a typical application:
 * 1. The application builds a GOptions object using gfields::defineOptions().
 * 2. The application constructs \ref GMagneto "GMagneto".
 * 3. The application attaches a \c G4FieldManager to a volume by looking up the desired field name.
 *
 * @section gfield_arch Architecture
 * @subsection gfield_arch_design Design notes
 * - **Plugin boundary:** concrete fields are implemented in shared libraries and must export a C symbol named \c GFieldFactory.
 * - **Configuration model:** field configuration is stored as strings in \ref GFieldDefinition::field_parameters "field_parameters"
 *   to preserve unit expressions; the concrete implementation parses/caches values in
 *   \ref GField::load_field_definitions "load_field_definitions()".
 * - **Stepping:** \ref GField::create_FieldManager "create_FieldManager()" creates the stepping components:
 *   \c G4Mag_UsualEqRhs, an integration stepper, a \c G4ChordFinder, and finally the \c G4FieldManager.
 *
 * @section options_sec Available Options and their usage
 *
 * This module reads the following option keys from the runtime option provider:
 *
 * - `gmultipoles`
 *   - Type: sequence of maps
 *   - Meaning: define one or more ideal multipole fields (each entry becomes one named field)
 *   - Behavior:
 *     - each entry is translated into a \ref GFieldDefinition "GFieldDefinition"
 *     - each definition is associated with the plugin type \c "multipoles"
 *     - the field implementation parses unit-bearing strings during field initialization
 *   - Subkeys used by this module:
 *     - `name` (string, mandatory): unique field key used by \ref GMagneto::getField "getField()"
 *     - `integration_stepper` (string): Geant4 integration stepper name
 *     - `minimum_step` (string): Geant4 length expression for the chord-finder minimum step
 *     - `pole_number` (string, mandatory): even integer >= 2 (2=dipole, 4=quadrupole, ...)
 *     - `vx`, `vy`, `vz` (string): origin coordinates (Geant4 length units)
 *     - `rotation_angle` (string): roll rotation about \c rotaxis (Geant4 angle units)
 *     - `rotaxis` (string, mandatory): one of X, Y, Z
 *     - `strength` (string, mandatory): field strength in Tesla (defined at 1 m reference radius for multipoles)
 *     - `longitudinal` (string/boolean-like): if \c true, return a uniform field aligned with \c rotaxis (solenoid-like)
 *
 * @section gfield_verbosity Module verbosity
 * The module uses the loggers \c gfield and \c gmagneto.
 * Typical verbosity behavior:
 * - Level 0: high-level summary and critical configuration messages.
 * - Level 1: field-definition and plugin-loading details (which fields were created, parsed parameters).
 * - Level 2: per-call or fine-grained diagnostic output (e.g. detailed field evaluation logging in some implementations).
 * - Debug: intended for intensive developer diagnostics (tight loops, repeated calls, full internal dumps).
 *
 * @section gfield_examples Examples
 * The module includes small example programs demonstrating usage.
 *
 * - \ref example_test_gfield_dipole : Minimal setup: define options, build \ref GMagneto "GMagneto", query a field, and evaluate B.
 *
 * \author
 * &copy; Maurizio Ungaro
 * \author e-mail: ungaro@jlab.org
 */
