#pragma once

namespace gstreamer {

/**
 * \file gstreamerConventions.h
 * \brief Shared constants and error codes for the gstreamer module.
 * \ingroup gstreamer_core_api
 */

/**
 * \brief Default number of buffered events before a GStreamer instance flushes its internal buffer.
 *
 * This value is used when the user does not override buffering through the \c ebuffer option.
 * The actual runtime limit is stored per streamer instance and may be updated from configuration
 * through \ref GStreamer::set_loggers "set_loggers()".
 */
inline constexpr int DEFAULT_GSTREAMER_BUFFER_FLUSH_LIMIT = 100;

/**
 * \name gstreamer error codes
 * \brief Error and diagnostic codes reserved for the gstreamer module.
 *
 * These values are grouped in the 800 range so gstreamer-related failures remain easy to identify
 * in logs and error reports.
 */
///@{
/// Plugin factory symbol could not be resolved.
inline constexpr int ERR_GSTREAMERFACTORYNOTFOUND = 801;
/// Duplicate variable registration was attempted in a streamer-specific schema.
inline constexpr int ERR_GSTREAMERVARIABLEEXISTS = 802;
/// Output medium could not be opened successfully.
inline constexpr int ERR_CANTOPENOUTPUT = 803;
/// Output medium could not be closed cleanly.
inline constexpr int ERR_CANTCLOSEOUTPUT = 804;
/// Publish sequence encountered invalid state or invalid input data.
inline constexpr int ERR_PUBLISH_ERROR = 805;
///@}

} // namespace gstreamer
