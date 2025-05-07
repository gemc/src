#pragma once

#define G4SYSTEMNATFACTORY "native"
#define G4SYSTEMCADFACTORY "cad"
#define G4SYSTEMGDMFACTORY "gdml"

// error codes in the 500s
#define ERR_G4DEPENDENCIESNOTSOLVED   501
#define ERR_G4WRONGORDEREDROTATION    502
#define ERR_G4PARAMETERSMISMATCH      503
#define ERR_G4SYSTEMFACTORYNOTFOUND   504
#define ERR_G4MATERIALNOTFOUND        505
#define ERR_G4SOLIDTYPENOTFOUND       506
#define ERR_G4VOLUMEBUILDFAILED       506

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


