#pragma once

/**
 * @defgroup gparticle_module gparticle
 * @brief Single-particle (and multi-particle) generator configuration and shooting utilities.
 *
 * The gparticle module provides:
 * - A lightweight particle specification container: \ref Gparticle
 * - Option parsing utilities to build one or more \ref Gparticle objects from structured CLI options
 *
 * @ingroup gparticle_topics
 */

/**
 * @defgroup gparticle_topics Topics
 * @brief High-level documentation topics for the gparticle module.
 */

/**
 * @defgroup gparticle_options_topic Options and configuration
 * @brief How gparticle is configured through structured options.
 * @ingroup gparticle_topics
 */

/**
 * @defgroup gparticle_examples_topic Examples
 * @brief Example programs demonstrating how to use the gparticle module.
 * @ingroup gparticle_topics
 */

/**
 * @file gparticleDoxy.h
 * @brief Doxygen entry point for the gparticle module documentation.
 *
 * This header contains the module-level documentation (mainpage, groups, and
 * example links). It is intentionally not included by the code.
 *
 * @ingroup gparticle_module
 */

/**
 * @mainpage gparticle Module
 *
 * @section gparticle_overview Overview
 *
 * The gparticle module implements a simple mechanism to define one or more
 * primary particles from structured options and then shoot them into a
 * \c G4Event using a \c G4ParticleGun.
 *
 * The central class is \ref Gparticle, which encapsulates:
 * - The particle identity (name and PDG id resolution)
 * - Multiplicity (how many primaries to shoot per event)
 * - Momentum/angle/vertex parameters
 * - Randomization models (uniform/gaussian/cosine/sphere) as provided by gutilities
 *
 * Option parsing is performed via the functions in the gparticle namespace.
 * Those functions translate a structured option node (e.g. \c -gparticle=...)
 * into a vector of \ref Gparticle instances.
 *
 * @section gparticle_verbosity Verbosity
 *
 * gparticle uses logging through GLogger. When the logger verbosity is:
 * - **0**: only essential messages (errors and minimal status) are printed
 * - **1**: adds high-level informational messages describing the selected configuration
 * - **2**: adds detailed informational output, including printing the full particle configuration
 * - **debug**: emits constructor/destructor and fine-grained tracing messages useful for development
 *
 * @section gparticle_toc Table of contents
 * - \ref gparticle_module
 * - \ref gparticle_options_topic
 * - \ref gparticle_examples_topic
 *
 * @section gparticle_examples Examples
 *
 * - \ref gparticle_example_main : Minimal program that parses gparticle options, builds particles,
 *   and initializes a \c G4RunManager with a basic physics list.
 *
 * @section gparticle_example_snippet Example snippet
 *
 * The following excerpt shows how the example program builds the options and retrieves particles:
 * @code
 * auto gopts = std::make_shared<GOptions>(argc, argv, gparticle::defineOptions());
 * auto log   = std::make_shared<GLogger>(gopts, FUNCTION_NAME, GPARTICLE_LOGGER);
 * auto particles = gparticle::getGParticles(gopts, log);
 * @endcode
 */
