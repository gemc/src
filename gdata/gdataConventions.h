#pragma once

/**
 * \file gdataConventions.h
 * \brief Defines constants and exit codes for the GData library.
 *
 * This file contains global constants used throughout the GData library,
 * including exit codes and identifier strings.
 *
 * \mainpage GData Library
 *
 * \section intro_sec Introduction
 * The GData library provides utilities and classes for handling simulation data,
 * including true hit information and digitized data. This file defines the conventions,
 * constants, and default values used by the library.
 *
 * \section conventions_sec Conventions
 * The constants below are used for error reporting and as string identifiers in various modules.
 */

constexpr int ERR_GSDETECTORNOTFOUND = 601;    ///< Exit code when a detector is not found.
constexpr int ERR_VARIABLENOTFOUND   = 602;      ///< Exit code when a variable is not found.
constexpr int ERR_WRONGPAYLOAD       = 603;      ///< Exit code when a payload is of the wrong size.

constexpr const char* CRATESTRINGID         = "crate";                ///< Identifier for crate.
constexpr const char* SLOTSTRINGID          = "slot";                 ///< Identifier for slot.
constexpr const char* CHANNELSTRINGID       = "channel";              ///< Identifier for channel.
constexpr const char* CHARGEATELECTRONICS   = "chargeAtElectronics";  ///< Identifier for charge at electronics.
constexpr const char* TIMEATELECTRONICS     = "timeAtElectronics";    ///< Identifier for time at electronics.
constexpr int         TIMEATELECTRONICSNOTDEFINED = -123456;         ///< Default value for undefined time at electronics.

