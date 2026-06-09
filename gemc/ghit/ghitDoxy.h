/**
* \mainpage ghit : Hit information utilities
 *
 * \section intro_sec Introduction
 * The \c ghit module provides a compact hit container (\c GHit) for storing step-by-step
 * and aggregated information from detector simulations.
 *
 * A \c GHit records per-step information unconditionally:
 * - global and local positions
 * - energy deposited (optionally scaled by detector-specific multipliers)
 * - global time
 * - track identity (PDG, track ID, parent track ID, parent PDG)
 * - track 3-momentum and total energy
 * - creator process name
 *
 * \section components_sec Components
 * - \c GHit : hit container that accumulates per-step vectors and provides lazy derived quantities.
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
