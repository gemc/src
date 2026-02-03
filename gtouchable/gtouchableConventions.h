#pragma once

/**
 * @file gtouchableConventions.h
 * @brief Conventions and constants used by the gtouchable module.
 *
 * This header centralizes:
 * - The digitization type strings used to classify a touchable.
 * - A sentinel value used for the readout time-cell index when it is not yet assigned.
 *
 */

/**
 * @brief Digitization type name for flux-like detectors.
 *
 * Flux touchables use the **track id** as the discriminating factor (in addition to the identity vector).
 * They also typically rely on standard "true infos" variables provided by the simulation.
 */
#define FLUXNAME "flux"

/**
 * @brief Digitization type name for simple particle counters.
 *
 * Particle-counter touchables require **no additional discriminating factor** beyond the identity vector.
 * They also typically rely on standard "true infos" variables provided by the simulation.
 */
#define COUNTERNAME "particle_counter"

/**
 * @brief Digitization type name for dosimeters.
 *
 * Dosimeter touchables use the **track id** as the discriminating factor (in addition to the identity vector),
 * and are typically associated with radiation digitization.
 */
#define DOSIMETERNAME "dosimeter"

/**
 * @brief Sentinel value for an unset electronics time-cell index.
 *
 * For \c readout touchables, the electronics time-cell index is used to decide whether two hits
 * belong to the same time window. Until the digitization plugin assigns a valid time-cell index,
 * this value is used to mark the index as "not set".
 */
#define GTOUCHABLEUNSETTIMEINDEX -1
