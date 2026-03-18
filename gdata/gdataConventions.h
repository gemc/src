#pragma once

/**
 * \file gdataConventions.h
 * \brief Shared constants, schema keys, and error codes for the GData module.
 *
 * \details
 * This header centralizes common key names and error codes used throughout the module so that:
 * - producers and consumers use identical observable names
 * - output backends can rely on stable key spelling
 * - diagnostics can rely on stable numeric codes
 *
 * The string constants declared here are part of the public schema contract of this module.
 * Changing one of these values affects interoperability with:
 * - detector plugins
 * - exporters
 * - validation tools
 * - downstream analysis code
 */

/**
 * \defgroup gdata_module_conventions GData module conventions
 * \brief Public constants, conventional key names, and stable error codes used across GData.
 *
 * \details
 * This topic groups the shared constants that define the public schema vocabulary for the module.
 * These values are intentionally centralized because they are consumed by multiple classes and by
 * downstream code that expects consistent key names and numeric diagnostics.
 */

/**
 * \name Error and reporting codes
 * \brief Stable numeric codes used by GData components when reporting failures.
 * \ingroup gdata_module_conventions
 *
 * \details
 * These values are intended to remain stable so that scripts and higher-level tools can classify
 * failures without depending on exact log-message text.
 * @{
 */
constexpr int ERR_GSDETECTORNOTFOUND = 601; ///< Requested sensitive detector entry is missing.
constexpr int ERR_VARIABLENOTFOUND   = 602; ///< Requested observable key is missing.
constexpr int ERR_WRONGPAYLOAD       = 603; ///< Packed payload vector has an unexpected size or layout.
/** @} */

/**
 * \name Streaming-readout key names
 * \brief Conventional observable keys used to represent electronics addressing and timing.
 * \ingroup gdata_module_conventions
 *
 * \details
 * These keys identify readout coordinates or closely related electronics-level quantities.
 * They are treated specially by the filtered accessors:
 * - GDigitizedData::getIntObservablesMap()
 * - GDigitizedData::getDblObservablesMap()
 *
 * This separation allows output code to distinguish between:
 * - channel-addressing information
 * - detector- or digitization-specific physics-like observables
 * @{
 */
constexpr const char* CRATESTRINGID       = "crate";   ///< Electronics crate identifier.
constexpr const char* SLOTSTRINGID        = "slot";    ///< Slot index within the crate.
constexpr const char* CHANNELSTRINGID     = "channel"; ///< Channel index within the slot or module.
constexpr const char* CHARGEATELECTRONICS = "chargeAtElectronics"; ///< Electronics-stage charge or ADC proxy.
constexpr const char* TIMEATELECTRONICS   = "timeAtElectronics";   ///< Electronics-stage time or TDC proxy.
/** @} */

/**
 * \brief Sentinel value returned when \c timeAtElectronics is requested but not present.
 * \ingroup gdata_module_conventions
 *
 * \details
 * This value is intentionally chosen to be unlikely in normal data so that missing-data situations
 * are easier to detect during debugging and validation.
 *
 * It is returned by GDigitizedData::getTimeAtElectronics().
 */
constexpr int TIMEATELECTRONICSNOTDEFINED = -123456;