#pragma once

namespace gtouchable {

/**
 * \file gtouchableConventions.h
 * \brief Conventions and constants used by the gtouchable module.
 *
 * This header centralizes:
 * - The digitization type strings used to classify a touchable.
 *
 */

/**
 * \brief Digitization type name for flux-like detectors.
 *
 * Flux touchables use the **track id** as the discriminating factor (in addition to the identity vector).
 * They also typically rely on standard "true infos" variables provided by the simulation.
 */
inline constexpr char FLUXNAME[] = "flux";

/**
 * \brief Digitization type name for optical-photon flux detectors.
 *
 * gPhotonDetector touchables use the **track id** as the discriminating factor, like flux,
 * but the digitization routine records only optical photons.
 */
inline constexpr char GPHOTON_DETECTORNAME[] = "gPhotonDetector";

/**
 * \brief Digitization type name for simple particle counters.
 *
 * Particle-counter  use the **particle id** as the discriminating factor (in addition to the identity vector).
 * They also typically rely on standard "true infos" variables provided by the simulation.
 */
inline constexpr char COUNTERNAME[] = "particle_counter";

/**
 * \brief Digitization type name for dosimeters.
 *
 * Dosimeter requires no additional discriminator beyond the identity vector. Meant to accumulate dose.
 */
inline constexpr char DOSIMETERNAME[] = "dosimeter";

/**
 * \brief Digitization type name for integral_counter.
 *
 * integral_counter requires no additional discriminator beyond the identity vector. It is meant to accumulate
 * all quantities within a sensitive cell
 */
inline constexpr char INTEGRAL_COUNTERNAME[] = "integral_counter";



} // namespace gtouchable
