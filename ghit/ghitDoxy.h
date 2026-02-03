/**
* \mainpage ghit : Hit information utilities
 *
 * \section intro_sec Introduction
 * The \c ghit module provides a compact hit container (\c GHit) and supporting conventions
 * (\c HitBitSet) for storing step-by-step and aggregated information from detector simulations.
 *
 * A \c GHit records always-present per-step information:
 * - global and local positions
 * - energy deposited (optionally scaled by detector-specific multipliers)
 * - global time
 *
 * It can also record optional per-step information controlled by \c HitBitSet (see ghitConventions.h :
 * bit meanings and reserved future extensions).
 *
 * \section components_sec Components
 * - \c GHit : hit container that accumulates per-step vectors and provides lazy derived quantities.
 * - \c HitBitSet : fixed-size bitset selecting optional hit information groups.
 * - ghitConventions.h : stable conventions for bit meanings and analysis expectations.
 *
 * \section examples_sec Examples
 * The module ships with an example program:
 * - \b ghit_example.cc : constructs a \c GTouchable, creates a \c GHit, generates randomized test hits,
 *   and compares them using \c GHit::is_same_hit().
 *
 * \section ownership_sec Ownership and contacts
 * Author: &copy; Maurizio Ungaro \n
 * e-mail: ungaro@jlab.org
 *
 * \note This module does not define verbosity levels because it does not introduce any classes derived
 *       from \c glogger. Logging in the provided example is performed using \c GLogger.
 */
