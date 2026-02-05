#pragma once

/**
 * \file gsystemConventions.h
 * \brief Conventions and shared constants for the detector-system module.
 *
 * This header centralizes:
 * - geometry/material row sizes used by database-backed factories;
 * - default placement/rotation strings for ROOT/world volumes;
 * - common error codes used by geometry/material loaders;
 * - factory labels and naming conventions.
 *
 */

// Assumptions:
// The volumes and materials in each system must have an unique name

/// \name Database row sizes
///@{
/// Number of database parameters defining a gvolume entry.
#define GVOLUMENUMBEROFPARS    21
/// Number of database parameters defining a gmaterial entry.
#define GMATERIALNUMBEROFPARS  18
///@}

/// \name Default placement strings (used primarily for the ROOT/world volume)
///@{
#define DEFAULTPOSITION "0*cm, 0*cm, 0*cm"
#define DEFAULTROTATION "0*deg, 0*deg, 0*deg"

///@}

/// \name Error codes (module-level exit/status codes)
/// \details Values are in the 200s range.
///@{
#define ERR_GVOLUMEALREADYPRESENT            201
#define ERR_GWRONGNUMBEROFPARS               202
#define ERR_GSETUPFILENOTOFOUND              203
#define ERR_GVOLUMENOTFOUND                  204
#define ERR_GDIRNOTFOUND                     205
#define ERR_GMATERIALOPTICALPROPERTYMISMATCH 206
#define ERR_GMATERIALNOTFOUND                207
#define ERR_GVOLUMENAMECONTAINSINVALID       208
#define ERR_GSQLITEERROR                     209
#define ERR_GMATERIALALREADYPRESENT          210
#define ERR_EXPERIMENTNOTFOUND               211
///@}

/// \name Modifier conventions
///@{
/// Special string used to represent “no modifier is requested”.
#define GSYSTEMNOMODIFIER          "noModifier"
///@}

/// \name Environment and naming conventions
///@{
/// Environment variable name used by GEMC database tooling.
#define GEMCDB_ENV           "GEMCDB_ENV"

/// Canonical name for the ROOT/world gvolume entry.
#define ROOTWORLDGVOLUMENAME "root"

/// Default ROOT/world volume definition string (type + dimensions + material).
/// \note The type corresponds to a Geant4 solid type. Use \c (not \ref) for external libraries.
#define ROOTDEFINITION       "G4Box 15*m 15*m 15*m G4_AIR"

/// Special mother-name marker for the top-level world root.
#define MOTHEROFUSALL        "akasha"

/// Delimiter used to build fully-qualified names (system/name).
#define GSYSTEM_DELIMITER    "/"
///@}

/// \name Factory labels
///@{
#define GSYSTEMASCIIFACTORYLABEL    "ascii"
#define GSYSTEMCADTFACTORYLABEL     "CAD"
#define GSYSTEMGDMLTFACTORYLABEL    "GDML"
#define GSYSTEMMYSQLTFACTORYLABEL   "mysql"
#define GSYSTEMSQLITETFACTORYLABEL  "sqlite"

/// Default sqlite DB filename used when the user does not specify one.
#define GSYSTEMSQLITETDEFAULTFILE   "gemc.db"

/// Default search directory for ASCII factory files.
#define GSYSTEMSASCIISEARCHDIR      "."
///@}
