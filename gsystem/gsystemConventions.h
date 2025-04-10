#ifndef  GSYSTEM_CONVENTIONS_H
#define  GSYSTEM_CONVENTIONS_H  1

// Assumptions:
// The volumes and materials in each system must have an unique name

// number of database pars defining a gvolume and a gmaterial
#define GVOLUMENUMBEROFPARS    21
#define GMATERIALNUMBEROFPARS  18

#define DEFAULTPOSITION       "0*cm, 0*cm, 0*cm";
#define DEFAULTROTATION       "0*deg, 0*deg, 0*deg";

// exit codes: 200s
#define EC__GVOLUMEALREADYPRESENT            201
#define EC__GWRONGNUMBEROFPARS               202
#define EC__GSETUPFILENOTOFOUND              203
#define EC__GVOLUMENOTFOUND                  204
#define EC__GDIRNOTFOUND                     205
#define EC__GMATERIALOPTICALPROPERTYMISMATCH 206
#define EC__GMATERIALNOTFOUND                207
#define EC__GVOLUMENAMECONTAINSINVALID       208
#define EC__GSQLITEERROR                     209

#define GSYSTEMLOGHEADER           " ❖ GSystem ▸ "
#define GSYSTEMNOMODIFIER          "noModifier"

#define GEMCDB_ENV           "GEMCDB_ENV"

#define ROOTWORLDGVOLUMENAME "root"
#define ROOTDEFINITION       "G4Box 15*m 15*m 15*m G4_AIR"
#define MOTHEROFUSALL        "akasha"
#define GSYSTEM_DELIMITER    "/"

// factory labels
#define GSYSTEMASCIIFACTORYLABEL    "ascii"
#define GSYSTEMCADTFACTORYLABEL     "CAD"
#define GSYSTEMGDMLTFACTORYLABEL    "GDML"
#define GSYSTEMMYSQLTFACTORYLABEL   "mysql"
#define GSYSTEMSQLITETFACTORYLABEL  "sqlite"
#define GSYSTEMSQLITETDEFAULTFILE   "gemc.db"

#endif
