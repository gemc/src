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
 * @section gfield_options Available Options and usage
 * The module currently defines one field option family implemented in this module:
 *
 * @subsection gfield_options_gmultipoles gmultipoles
 * Use \c gmultipoles to define one or more ideal multipole fields. Each entry becomes one named field loaded via a plugin.
 *
 * Typical fields:
 * - \c name : unique field key used by \ref GMagneto::getField "getField()" and \ref GMagneto::getFieldMgr "getFieldMgr()".
 * - \c integration_stepper : string name of a supported stepper (default: \c G4DormandPrince745).
 * - \c minimum_step : minimum step for the \c G4ChordFinder.
 * - \c pole_number : even integer >= 2 (2=dipole, 4=quadrupole, ...).
 * - \c vx, \c vy, \c vz : origin in Geant4 length units.
 * - \c rotation_angle : roll angle about \c rotaxis.
 * - \c rotaxis : one of X, Y, Z.
 * - \c strength : Tesla (defined at 1 m reference radius for multipoles).
 * - \c longitudinal : if \c true, return a uniform axial field aligned with \c rotaxis (solenoid-like).
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
