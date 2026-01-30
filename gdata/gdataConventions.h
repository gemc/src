#pragma once

/**
 * \file gdataConventions.h
 * \brief Shared constants and error codes for the GData library.
 *
 * The GData library passes observables around using string keys (e.g. "adc", "tdc",
 * "crate", "slot", ...). This header centralizes the most common identifiers used
 * across event/run/frame collectors to avoid spelling drift and to make filtering
 * deterministic.
 *
 * The error codes are used with \c GLogger::error(...) to provide stable exit/status
 * codes for higher-level tooling.
 */

/**
 * \name Error / exit codes
 * \brief Numeric error codes used by GData components for consistent reporting.
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
 * These keys are treated specially by \ref GDigitizedData::getIntObservablesMap() and
 * \ref GDigitizedData::getDblObservablesMap() when filtering "streaming readout"
 * variables (crate/slot/channel + electronics time/charge).
 * @{
 */
constexpr const char* CRATESTRINGID         = "crate";              ///< Electronics crate index.
constexpr const char* SLOTSTRINGID          = "slot";               ///< Slot index within a crate (typically module position).
constexpr const char* CHANNELSTRINGID       = "channel";            ///< Channel index within a slot/module.
constexpr const char* CHARGEATELECTRONICS   = "chargeAtElectronics";///< Charge (or ADC-integrated proxy) at electronics stage.
constexpr const char* TIMEATELECTRONICS     = "timeAtElectronics";  ///< Time (or TDC proxy) at electronics stage.
/** @} */

/**
 * \brief Sentinel value returned when \ref TIMEATELECTRONICS is requested but not present.
 *
 * This is intentionally an "unlikely" value to help catch missing-data bugs quickly.
 */
constexpr int TIMEATELECTRONICSNOTDEFINED = -123456;
