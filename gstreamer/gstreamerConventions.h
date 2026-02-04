#pragma once

/**
 * \file gstreamerConventions.h
 * \brief Shared constants and error codes for the gstreamer module.
 */

/**
 * \brief Default number of buffered events before a \ref GStreamer flushes its internal event buffer.
 *
 * This value is used as a sensible default when no user override is provided via options.
 * Concrete streamers may override the limit at runtime by calling \ref GStreamer::set_loggers "set_loggers()".
 */
# define DEFAULT_GSTREAMER_BUFFER_FLUSH_LIMIT 100

/**
 * \name gstreamer error codes
 * \brief Exit / error codes used by gstreamer components.
 *
 * These codes are in the 800 range to keep them grouped with other gstreamer-related diagnostics.
 */
///@{
#define ERR_GSTREAMERFACTORYNOTFOUND  801  ///< Plugin factory symbol not found or could not be resolved.
#define ERR_GSTREAMERVARIABLEEXISTS   802  ///< Attempt to register the same output variable twice (ROOT tree branch conflict).

#define ERR_CANTOPENOUTPUT            803  ///< Output medium could not be opened (file/device not accessible).
#define ERR_CANTCLOSEOUTPUT           804  ///< Output medium could not be closed cleanly.
#define ERR_PUBLISH_ERROR             805  ///< Generic publish-time error (null pointers, invalid state).
///@}
