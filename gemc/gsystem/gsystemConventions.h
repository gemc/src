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
 * Assumptions:
 * - Volumes and materials in each system must have a unique name.
 */

// -----------------------------------------------------------------------------
// Database row sizes
// -----------------------------------------------------------------------------

/**
 * \name Database row sizes
 * \brief Expected number of positional fields for serialized DB/ASCII rows.
 *
 * The module reads geometry and material data as serialized, positional vectors.
 * These constants define the expected number of fields for each record type.
 */
///@{
/// Number of database parameters defining a gvolume entry.
#define GVOLUMENUMBEROFPARS    21
/// Number of database parameters defining a gmaterial entry.
#define GMATERIALNUMBEROFPARS  18
///@}

// -----------------------------------------------------------------------------
// Default placement strings
// -----------------------------------------------------------------------------

/**
 * \name Default placement strings
 * \brief Default position and rotation strings, primarily for the ROOT/world volume.
 *
 * These are used when creating the implicit ROOT/world volume or when a configuration
 * requests a default placement. Units are embedded in the strings and interpreted by
 * the same utilities used by geometry parsing.
 */
///@{
#define DEFAULTPOSITION "0*cm, 0*cm, 0*cm"
#define DEFAULTROTATION "0*deg, 0*deg, 0*deg"
///@}

// -----------------------------------------------------------------------------
// Error codes
// -----------------------------------------------------------------------------

/**
 * \name Error codes
 * \brief Module-level exit/status codes.
 *
 * Values are in the 200s range and are used by the logger error path to provide
 * stable identifiers for common failure modes.
 */
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

// -----------------------------------------------------------------------------
// Modifier conventions
// -----------------------------------------------------------------------------

/**
 * \name Modifier conventions
 * \brief Sentinel values used when no modifier is requested.
 */
///@{
/// Special string used to represent “no modifier is requested”.
#define GSYSTEMNOMODIFIER          "noModifier"
///@}

// -----------------------------------------------------------------------------
// Environment and naming conventions
// -----------------------------------------------------------------------------

/**
 * \name Environment and naming conventions
 * \brief Shared names used across factories and core types.
 */
///@{
/// Environment variable name used by GEMC database tooling.
#define GEMCDB_ENV           "GEMCDB_ENV"

/// Canonical name for the ROOT/world gvolume entry.
#define ROOTWORLDGVOLUMENAME "root"

/// Default ROOT/world volume definition string (type + dimensions + material).
/// \note The type corresponds to a Geant4 solid type string (e.g. \c G4Box).
#define ROOTDEFINITION       "G4Box 15*m 15*m 15*m G4_AIR"

/// Special mother-name marker for the top-level world root.
#define MOTHEROFUSALL        "akasha"

/// Delimiter used to build fully-qualified names (system/name).
#define GSYSTEM_DELIMITER    "/"
///@}

// -----------------------------------------------------------------------------
// Factory labels
// -----------------------------------------------------------------------------

/**
 * \name Factory labels
 * \brief Identifiers used in configuration to select concrete factories.
 */
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
