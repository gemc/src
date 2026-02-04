#pragma once

// gemc
#include "goptions.h"

// c++
#include <vector>

/**
 * \file gstreamer_options.h
 * \brief Option and configuration helpers for the gstreamer module.
 */

/**
 * \brief Logger category name used by gstreamer components.
 *
 * This string is passed to the GEMC logging system so that verbosity and routing can be configured
 * consistently for the entire module.
 */
constexpr const char* GSTREAMER_LOGGER = "gstreamer";

/**
 * \brief Utility struct describing one configured output for the gstreamer module.
 *
 * A \ref GStreamerDefinition is typically produced from a user option node and then specialized
 * per-thread by appending a thread suffix to the base filename.
 *
 * Members:
 * - \ref format : output format selector (e.g. \c "root", \c "ascii", \c "jlabsro", \c "csv").
 * - \ref rootname : user-provided base filename (without extension). A per-thread suffix may be appended.
 * - \ref type : semantic output type (e.g. \c "event" or \c "stream"). Plugins may use this to select
 *   which hooks are implemented.
 * - \ref tid : thread id used to specialize the filename; negative means "not thread specialized".
 */
struct GStreamerDefinition
{
	/// \brief Default constructor producing an empty definition.
	GStreamerDefinition() = default;

	/**
	 * \brief Construct from explicit fields.
	 * \param f Output format token.
	 * \param n Base filename (without extension).
	 * \param t Output type token.
	 */
	GStreamerDefinition(std::string f, std::string n, std::string t) :
		format(std::move(f)), rootname(std::move(n)), type(std::move(t)) {
	}

	/**
	 * \brief Construct a per-thread variant of an existing definition.
	 *
	 * If \p t is non-negative, the constructor appends \c "_t<tid>" to \ref rootname.
	 * If \p t is negative, the original \ref rootname is preserved and \ref tid remains negative.
	 *
	 * \param other Source definition (usually shared across all threads).
	 * \param t Thread id. Use a negative value to disable filename specialization.
	 */
	GStreamerDefinition(const GStreamerDefinition& other, int t) :
		format(other.format), rootname(other.rootname + "_t" + std::to_string(t)), type(other.type), tid(t) {
		if (tid < 0) {
			rootname = other.rootname; // if tid is negative, use the original rootname
		}
	}

	/// \brief Output format token used to select a plugin.
	std::string format;

	/// \brief Base filename (without extension), possibly specialized by thread id.
	std::string rootname;

	/// \brief Semantic output type token (e.g. \c "event" or \c "stream").
	std::string type;

	/// \brief Thread id used to specialize \ref rootname; negative means "not specialized".
	int tid = -1;

	/**
	 * \brief Return the plugin library / object name used by the dynamic loader.
	 *
	 * The convention is: \c "gstreamer_<format>_plugin".
	 *
	 * \return The plugin name derived from \ref format.
	 */
	[[nodiscard]] std::string gstreamerPluginName() const { return "gstreamer_" + format + "_plugin"; }
};


namespace gstreamer {
/**
 * \brief Parse gstreamer output definitions from options.
 *
 * This reads the \c "gstreamer" option node and creates a list of \ref GStreamerDefinition objects.
 * Each entry is expected to contain:
 * - \c format : plugin format token.
 * - \c filename : base output filename (without extension).
 * - \c type : output type token (defaults to \c "event" when omitted).
 *
 * \param gopts Options container.
 * \return Vector of configured output definitions.
 */
std::vector<GStreamerDefinition> getGStreamerDefinition(const std::shared_ptr<GOptions>& gopts);

/**
 * \brief Contribute gstreamer options to the global option set.
 *
 * The returned GOptions aggregates:
 * - gstreamer options (buffer settings and output definitions)
 * - options from dependent modules (e.g. gdynamicdigitization)
 *
 * \return A fully populated GOptions instance for this module.
 */
GOptions defineOptions();
}
