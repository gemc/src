#pragma once

namespace g4system {
/**
 * \file   g4systemConventions.h
 * @ingroup g4system
 * \brief  Conventions, labels, and error codes used by the g4system geometry/material layer.
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
inline constexpr char G4SYSTEMNATFACTORY[] = "native";
inline constexpr char G4SYSTEMCADFACTORY[] = "cad";
inline constexpr char G4SYSTEMGDMFACTORY[] = "gdml";

// error codes in the 500s
inline constexpr int ERR_G4DEPENDENCIESNOTSOLVED =
    501; ///< Geometry/material dependencies could not be resolved.
inline constexpr int ERR_G4WRONGORDEREDROTATION = 502; ///< Rotation specification was malformed or unsupported.
inline constexpr int ERR_G4PARAMETERSMISMATCH =
    503; ///< Solid parameter count/format did not match expected constructors.
inline constexpr int ERR_G4SYSTEMFACTORYNOTFOUND =
    504;                                             ///< A required Geant4 system factory was not found/mapped.
inline constexpr int ERR_G4MATERIALNOTFOUND = 505;   ///< Material lookup failed and no fallback was available.
inline constexpr int ERR_G4SOLIDTYPENOTFOUND =
    506; ///< Requested solid type is not supported by the native factory.
inline constexpr int ERR_G4VOLUMEBUILDFAILED =
    506; ///< A volume could not be fully built (solid/logical/physical).
inline constexpr int ERR_G4MIRRORNOTFOUND =
    507; ///< A volume references a mirror that is not defined in its system.
inline constexpr int ERR_G4SURFACECONFIGINVALID =
    508; ///< Optical surface type/finish/model/properties are invalid.

// Special value used by options to indicate "no fallback material".
inline constexpr char NO_USE_DEFAULT_MATERIAL[] = "noDefaultMaterial";

// predefined isotopes / elements / materials
inline constexpr char HYDROGEN_ELEMENT[] = "Hydrogen";
inline constexpr char DEUTERON_ISOTOPE[] = "Deuteron";
inline constexpr char DEUTERIUM_ELEMENT[] = "Deuterium";
inline constexpr char HGAS_MATERIAL[] = "HydrogenGas";
inline constexpr char DEUTERIUMGAS_MATERIAL[] = "DeuteriumGas";
inline constexpr char LD2_MATERIAL[] = "LD2";
inline constexpr char NITRO_ELEMENT[] = "Nitrogen";
inline constexpr char ND3_MATERIAL[] = "ND3";
inline constexpr char HELION_ISOTOPE[] = "Helion";
inline constexpr char HELIUM3_ELEMENT[] = "Helium3";
inline constexpr char HELIUM3GAS_MATERIAL[] = "Helium3Gas";
inline constexpr char TRITON_ISOTOPE[] = "Triton";
inline constexpr char TRITIUM_ELEMENT[] = "Tritium";
inline constexpr char TRITIUMGAS_MATERIAL[] = "H3Gas";
inline constexpr char AIROPTICAL_MATERIAL[] = "G4_AIR_Optical";
inline constexpr char KRYPTONITE_MATERIAL[] = "Kryptonite";

} // namespace g4system
