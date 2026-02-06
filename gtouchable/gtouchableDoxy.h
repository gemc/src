/// \mainpage gtouchable module
///
/// \section gtouchable_overview Overview
/// The \c gtouchable module provides a compact representation of a *sensitive detector element* that can be
/// used as a **key** when building and merging hit collections during digitization.
///
/// A \c GTouchable is uniquely described by:
/// - A list of identifiers (the *identity vector*), e.g. \c "sector: 2, layer: 4, wire: 33".
/// - A discriminator rule that depends on the touchable type (readout/flux/particleCounter/dosimeter).
///
/// Conceptually, a touchable is the “address” of a detector element *plus* the extra context required to decide
/// whether two hits belong to the same readout cell (and therefore can be merged).
///
/// \section gtouchable_detector_types Main detector types
/// The module supports the following touchable types:
/// - \c readout : electronic time window is the discriminating factor in addition to the identity vector.
/// - \c flux : track id is the discriminating factor in addition to the identity vector.
/// - \c particleCounter : the identity vector is sufficient (no additional discriminating factor).
/// - \c dosimeter : track id is the discriminating factor in addition to the identity vector (radiation digitization).
///
/// \section options_sec Available Options and their usage
///
/// This module currently does not define or consume any module-specific option keys.
///
/// Notes:
/// - The module participates in the standard logging configuration via \c TOUCHABLE_LOGGER.
/// - Global keys defined by \ref GOptions::GOptions "GOptions(argc,argv,...)" (e.g. `verbosity`, `debug`)
///   may influence logger behavior for this module, but no runtime configuration keys are read by the data structure.
///
/// \section gtouchable_examples Examples
/// \tableofcontents
///
/// \subsection gtouchable_example_basic gtouchable_example.cc
/// Demonstrates:
/// - Constructing a \c GTouchable from an identity string.
/// - Creating test touchables with \c GTouchable::create().
/// - Comparing two touchables using \c operator== and logging the result.
///
/// Source file:
/// - \c examples/gtouchable_example.cc
///
/// \section gtouchable_ownership Ownership and extension points
/// - Ownership: This module is part of GEMC and is maintained within the GEMC codebase.
/// - Extension points: The rules that assign readout timing, energy multipliers, and other digitization-dependent
///   attributes are typically implemented in digitization plugins. This module focuses on the data structure and
///   comparison semantics used by the hit processing pipeline.
///
/// \section gtouchable_verbosity Verbosity and logging
/// The module uses the \c GLogger infrastructure via the logger name \c "gtouchable" (see \c TOUCHABLE_LOGGER).
/// Typical verbosity behavior:
/// - Level 0: important messages only (rare in this module).
/// - Level 1: high-level informational messages for normal workflows.
/// - Level 2: detailed informational messages, typically used for validation and troubleshooting (e.g. existence checks).
/// - Debug: step-by-step internal diagnostics (e.g. constructor traces and per-identifier comparisons).
///
/// \author \n &copy; Maurizio Ungaro
/// \author e-mail: ungaro@jlab.org
///
