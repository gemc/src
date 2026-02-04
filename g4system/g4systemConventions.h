#pragma once
/**
 * @file   g4systemConventions.h
 * @ingroup Geometry
 * @brief  Conventions, labels, and error codes used by the g4system geometry/material layer.
 *
 * @details
 * This header centralizes:
 * - factory labels used to route systems to the proper Geant4 object factory
 * - error codes emitted through the module logger
 * - names of predefined isotopes/elements/materials created at startup
 *
 * Keeping these symbols here avoids hard-coded strings sprinkled across builders and factories.
 */

// factory labels
#define G4SYSTEMNATFACTORY "native"
#define G4SYSTEMCADFACTORY "cad"
#define G4SYSTEMGDMFACTORY "gdml"

// error codes in the 500s
#define ERR_G4DEPENDENCIESNOTSOLVED   501   ///< Geometry/material dependencies could not be resolved.
#define ERR_G4WRONGORDEREDROTATION    502   ///< Rotation specification was malformed or unsupported.
#define ERR_G4PARAMETERSMISMATCH      503   ///< Solid parameter count/format did not match expected constructors.
#define ERR_G4SYSTEMFACTORYNOTFOUND   504   ///< A required Geant4 system factory was not found/mapped.
#define ERR_G4MATERIALNOTFOUND        505   ///< Material lookup failed and no fallback was available.
#define ERR_G4SOLIDTYPENOTFOUND       506   ///< Requested solid type is not supported by the native factory.
#define ERR_G4VOLUMEBUILDFAILED       506   ///< A volume could not be fully built (solid/logical/physical).

// Special value used by options to indicate "no fallback material".
#define NO_USE_DEFAULT_MATERIAL "noDefaultMaterial"

// predefined isotopes / elements / materials
#define HYDROGEN_ELEMENT      "Hydrogen"
#define DEUTERON_ISOTOPE      "Deuteron"
#define DEUTERIUM_ELEMENT     "Deuterium"
#define HGAS_MATERIAL         "HydrogenGas"
#define DEUTERIUMGAS_MATERIAL "DeuteriumGas"
#define LD2_MATERIAL          "LD2"
#define NITRO_ELEMENT         "Nitrogen"
#define ND3_MATERIAL          "ND3"
#define HELION_ISOTOPE        "Helion"
#define HELIUM3_ELEMENT       "Helium3"
#define HELIUM3GAS_MATERIAL   "Helium3Gas"
#define TRITON_ISOTOPE        "Triton"
#define TRITIUM_ELEMENT       "Tritium"
#define TRITIUMGAS_MATERIAL   "H3Gas"
