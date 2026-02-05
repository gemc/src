#pragma once

/**
 * @defgroup gparticle_module gparticle
 * @brief Single-particle (and multi-particle) generator configuration and shooting utilities.
 *
 * @ingroup gparticle_topics
 */

/**
 * @defgroup gparticle_topics Topics
 * @brief High-level documentation topics for the gparticle module.
 */

/**
 * @file gparticleConventions.h
 * @brief Conventions and error codes for the gparticle module.
 *
 * @ingroup gparticle_module
 */

/**
 * @brief Sentinel value used to mark an unset/undefined numeric parameter.
 *
 * @ingroup gparticle_module
 */
#define GPARTICLENOTDEFINED (-999)

/**
 * @name Error codes
 * @brief Error codes emitted by gparticle through GLogger.
 *
 * Values are in the 900s to reduce collision with other modules.
 * @{
 * @ingroup gparticle_module
 */

/** @brief Requested particle name was not found in the \c G4ParticleTable. */
#define ERR_GPARTICLENOTFOUND        901

/** @brief Theta randomization model was not recognized or not supported. */
#define ERR_GPARTICLEWRONGTHETAMODEL 902

/** @brief \c G4ParticleTable could not be obtained (unexpected runtime state). */
#define ERR_GPARTICLETABLENOTFOUND   903

/** @} */
