#pragma once

// c++
#include <bitset>

/**
 * \file ghitConventions.h
 * \brief Defines the hit information selection bitset for \c GHit.
 *
 * The hit system records a core set of per-step quantities unconditionally and can
 * optionally record additional per-step information controlled by a fixed-size bitset.
 *
 * Always recorded (independent of the bitset):
 * - local and global positions (\c G4ThreeVector)
 * - energy deposited and time
 * - detector dimensions (via \c GTouchable)
 *
 * Optional information is controlled by \c HitBitSet, with one bit per feature group.
 * The bit meanings below define the expected mapping and help keep binary compatibility
 * and analysis conventions stable.
 */

// number of bits that defines a ghit.
constexpr size_t NHITBITS = 6;

/**
 * \brief Bitset selecting which optional hit information is recorded.
 *
 * \note The bit index meanings are documented below. Some higher bits are reserved
 *       for future extensions and may not yet be populated by the current implementation.
 */
using HitBitSet = std::bitset<NHITBITS>;

// -----------------------------------------------------------------------------
// HitBitSet conventions
// -----------------------------------------------------------------------------
//
// Setting a bit enables recording of the corresponding optional information.
// The current implementation fills only a subset of these planned fields.
//
// Bit 0 (mask 000001):
//   - Per-step particle IDs (PDG encoding)
//   - Per-step total energies (Es)
//   - Per-step creator process names (when available)
//
// Bit 1 (mask 000010) [planned]:
//   - Step length
//   - Track information (momentum, total energy, vertex, pid, track id)
//
// Bit 2 (mask 000100) [planned]:
//   - Mother particle track information
//
// Bit 3 (mask 001000) [planned]:
//   - Meta information (e.g. magnetic field sampling, process IDs)
//   - Process ID/name that created the particle
//
// Bit 4 (mask 010000) [planned]:
//   - Optical-photon-specific information (wavelength, momentum, vertex, track id)
//
// Bit 5 (mask 100000) [reserved]:
//   - Reserved for future use; keep stable for backward compatibility.
//
