#pragma once

/**
 * \file gdataConventions.h
 * \brief Shared constants and error codes for the GData library.
 *
 * \details
 * The GData library stores and transports observables using string keys (e.g. "adc", "tdc",
 * "crate", "slot", ...). This header centralizes commonly used identifiers to:
 * - prevent spelling drift across modules
 * - allow deterministic filtering/export behavior
 * - provide stable, documented keys for plugins and backends
 *
 * It also defines numeric error codes used with \ref GLogger::error(...) so that higher-level
 * tooling can interpret failure modes in a stable way.
 */

/**
 * \name Error / exit codes
 * \brief Numeric error codes used by GData components for consistent reporting.
 *
 * \details
 * These codes are intended to be stable across releases so that scripts, wrappers, and
 * downstream applications can classify failures.
 * @{
 */
constexpr int ERR_GSDETECTORNOTFOUND = 601; ///< Requested sensitive detector is missing from a collection/map.
constexpr int ERR_VARIABLENOTFOUND   = 602; ///< Requested observable key is missing from an observables map.
constexpr int ERR_WRONGPAYLOAD       = 603; ///< A payload vector has the wrong size/shape for the expected format.
/** @} */

/**
 * \name Streaming / readout identifiers (SRO keys)
 * \brief Conventional keys used to label electronics readout coordinates and timing.
 *
 * \details
 * "SRO" (streaming readout) keys are treated specially by some APIs.
 *
 * For example, \ref GDigitizedData::getIntObservablesMap() and
 * \ref GDigitizedData::getDblObservablesMap() accept a \c which argument that allows:
 * - selecting only SRO keys (crate/slot/channel/time/charge)
 * - selecting only non-SRO keys (physics-like digitized observables)
 *
 * This enables output backends to store readout addressing separately from digitization results.
 * @{
 */
constexpr const char* CRATESTRINGID         = "crate";              ///< Electronics crate index.
constexpr const char* SLOTSTRINGID          = "slot";               ///< Slot index within a crate (module position).
constexpr const char* CHANNELSTRINGID       = "channel";            ///< Channel index within a slot/module.
constexpr const char* CHARGEATELECTRONICS   = "chargeAtElectronics";///< Charge (or ADC-integrated proxy) at electronics stage.
constexpr const char* TIMEATELECTRONICS     = "timeAtElectronics";  ///< Time (or TDC proxy) at electronics stage.
/** @} */

/**
 * \brief Sentinel value returned when \ref TIMEATELECTRONICS is requested but not present.
 *
 * \details
 * This is intentionally an "unlikely" value to help catch missing-data bugs quickly.
 * Used by \ref GDigitizedData::getTimeAtElectronics().
 */
constexpr int TIMEATELECTRONICSNOTDEFINED = -123456;
