#pragma once

/**
 * @defgroup gparticle_options_topic Options and configuration
 * @brief How gparticle is configured through structured options.
 *
 * @ingroup gparticle_topics
 */

// gparticle
#include "gparticle.h"

// gemc
#include "goptions.h"

constexpr const char* GPARTICLE_LOGGER = "gparticle";

/**
 * @file gparticle_options.h
 * @brief Public API for defining and parsing gparticle-related options.
 *
 * This header contains the authoritative documentation for the option utilities.
 * The corresponding implementation file only provides a short non-Doxygen
 * summary per function (to avoid duplication between .h and .cc).
 *
 * @ingroup gparticle_options_topic
 */

namespace gparticle {
/**
 * @brief Defines the structured options used by the gparticle module.
 *
 * This function returns a fully populated GOptions instance containing the
 * option specification for \c -gparticle.
 *
 * The \c -gparticle option is expected to represent a list of particle
 * definitions. Each list item maps keys (e.g. \c name, \c p, \c theta, \c vx)
 * to typed values, which are then used to construct \ref Gparticle objects.
 *
 * The help text produced by this option definition includes usage guidance
 * and examples of the supported syntax.
 *
 * @return A GOptions instance containing the gparticle option schema.
 *
 * @ingroup gparticle_options_topic
 */
GOptions defineOptions();

/**
 * @brief Builds the list of generator particles from structured options.
 *
 * This function reads the \c gparticle option node from the provided GOptions
 * instance and creates a \c std::vector of \ref Gparticle shared pointers.
 *
 * For each entry in the structured node:
 * - mandatory fields are validated (e.g. \c name)
 * - numeric values are interpreted together with their unit strings
 * - randomization model strings are converted into gutilities::randomModel
 *
 * The provided logger is passed to each \ref Gparticle instance to ensure that
 * particle-level diagnostics are emitted consistently.
 *
 * @param gopts The parsed option container that holds the \c gparticle node.
 * @param logger Logger used for diagnostics and propagated into each particle.
 *
 * @return Vector of \ref Gparticle instances representing the configured primaries.
 *
 * @ingroup gparticle_options_topic
 */
std::vector<GparticlePtr> getGParticles(const std::shared_ptr<GOptions>& gopts,
                                        std::shared_ptr<GLogger>&        logger);
} // namespace gparticle
