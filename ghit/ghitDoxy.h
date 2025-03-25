/**
 * \mainpage GEMC Hit and Detector Information Library
 *
 * \section intro_sec Introduction
 * This library provides classes and utilities for handling hit information
 * in Geant4-based detector simulations. It supports detailed hit recording,
 * configurable hit information via bit masks, and integration with Geant4
 * visualization.
 *
 * \section features_sec Features
 * - Configurable hit information using a bitset (HitBitSet).
 * - Support for standard hit data such as positions, energy deposit, and time.
 * - Additional information (e.g., particle IDs, process names) toggled by bit flags.
 *
 * \section usage_sec Usage
 * - Use GHit to represent a detector hit.
 * - Use GTouchable to identify sensitive detector elements.
 *
 * \n\n
 * \author \n &copy; Maurizio Ungaro
 * \author e-mail: ungaro@jlab.org\n\n\n
 */


/**
 * \mainpage Geant4 Hit Information Conventions
 *
 * \section intro_sec Introduction
 * This module defines the conventions for representing hit information in a
 * Geant4 simulation. It uses a bitset (HitBitSet) to select which quantities
 * should be recorded for each hit.
 *
 * \section details_sec Details
 * The bitset controls the inclusion of standard quantities (positions,
 * energy deposition, time) as well as additional information such as particle IDs,
 * process names, and step length.
 *
 * \n\n
 * \author \n &copy; Maurizio Ungaro
 * \author e-mail: ungaro@jlab.org\n\n\n
 */


/**
 * \file ghitConventions.h
 * \brief Defines the HitBitSet and associated bit indices.
 */


/**
 * \file ghitConventions.h
 * \brief Defines the hit conventions and the HitBitSet for GHit.
 */
