/**
 * \mainpage ghit : Hit information utilities
 *
 * \tableofcontents
 *
 * \image html ghit-flow.svg "Geant4 step accumulation into a GEMC hit" width=900px
 *
 * \section ghit_intro Introduction
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
 * \section ghit_visual_model Visual model
 * Steps with the same detector-cell identity and discriminator accumulate into one \c GHit. Step positions,
 * times, energy deposits, and track metadata remain aligned by vector index, while steps in neighboring cells
 * or with a different discriminator form separate hits.
 *
 * \image html ghit-detector-steps.svg "Tracks and steps crossing a segmented sensitive detector" width=900px
 *
 * Derived values, such as total deposited energy and time extent, are calculated from the retained step
 * vectors. Digitizers can therefore inspect either the raw step sequence or the summarized hit view.
 *
 * \image html ghit-accumulation.svg "Per-step vectors and derived GHit quantities" width=900px
 *
 * \section ghit_components Components
 * - \c GHit : hit container that accumulates per-step vectors and provides lazy derived quantities.
 *
 * \section ghit_examples Examples
 * The module ships with an example program:
 * - \b ghit_example.cc : constructs a \c GTouchable, creates a \c GHit, generates randomized test hits,
 *   and compares them using \c GHit::is_same_hit().
 *
 * \section ghit_ownership Ownership and contacts
 * Author: &copy; Maurizio Ungaro \n
 * e-mail: ungaro@jlab.org
 *
 * \note This module does not define verbosity levels because it does not introduce any classes derived
 *       from \c glogger. Logging in the provided example is performed using \c GLogger.
 */
