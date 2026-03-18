#pragma once

// gemc
#include "goptions.h"

// c++
#include <vector>

/**
 * \file gstreamer_options.h
 * \brief Option and configuration helpers for the gstreamer module.
 * \ingroup gstreamer_options_api
 */

/**
 * \brief Logger category name used by gstreamer components.
 *
 * This string identifies the module in the logging system so that all classes and helper
 * functions participating in gstreamer output can be configured consistently for verbosity
 * and routing.
 */
constexpr const char* GSTREAMER_LOGGER = "gstreamer";

/**
 * \struct GStreamerDefinition
 * \ingroup gstreamer_options_api
 * \brief Lightweight description of one configured gstreamer output.
 *
 * A streamer definition is the normalized representation of one object inside the
 * \c -gstreamer option node. It captures:
 * - the output \ref format plugin selector
 * - the output \ref rootname base name
 * - the semantic \ref type of data to be written
 * - the optional \ref tid used to specialize filenames in multithreaded execution
 *
 * The struct does not own any file or plugin resources. It is purely a value object used during
 * configuration parsing and streamer instantiation.
 */
struct GStreamerDefinition
{
	/// \brief Default constructor producing an empty definition.
	GStreamerDefinition() = default;

	/**
	 * \brief Construct a definition from explicit field values.
	 *
	 * \param f Output format token used to choose the plugin implementation.
	 * \param n Base output filename without extension.
	 * \param t Semantic output type such as \c "event" or \c "stream".
	 */
	GStreamerDefinition(std::string f, std::string n, std::string t) :
		format(std::move(f)), rootname(std::move(n)), type(std::move(t)) {
	}

	/**
	 * \brief Construct a per-thread specialization from an existing definition.
	 *
	 * When \p t is non-negative, the constructor appends \c "_t<tid>" to the source
	 * \ref rootname so that each worker thread naturally writes to a distinct output name.
	 * When \p t is negative, the original base name is preserved unchanged.
	 *
	 * \param other Source definition to copy.
	 * \param t Worker thread identifier. A negative value disables filename specialization.
	 */
	GStreamerDefinition(const GStreamerDefinition& other, int t) :
		format(other.format), rootname(other.rootname + "_t" + std::to_string(t)), type(other.type), tid(t) {
		if (tid < 0) {
			rootname = other.rootname;
		}
	}

	/// \brief Output format token used to select the plugin implementation.
	std::string format;

	/// \brief Base output filename without extension, optionally specialized by thread id.
	std::string rootname;

	/// \brief Semantic output type, typically \c "event" or \c "stream".
	std::string type;

	/// \brief Worker thread id associated with this definition, or a negative value when not specialized.
	int tid = -1;

	/**
	 * \brief Return the plugin library name expected by the dynamic loader.
	 *
	 * The naming convention used by this module is:
	 * \code
	 * gstreamer_<format>_plugin
	 * \endcode
	 *
	 * \return Plugin object name derived from the current \ref format field.
	 */
	[[nodiscard]] std::string gstreamerPluginName() const { return "gstreamer_" + format + "_plugin"; }
};


namespace gstreamer {

/**
 * \ingroup gstreamer_options_api
 * \brief Parse all configured gstreamer output definitions from the options container.
 *
 * This function reads the \c "gstreamer" option node and converts each object entry into a
 * \ref GStreamerDefinition value. Each entry is expected to provide:
 * - \c format : plugin format selector
 * - \c filename : base output filename
 * - \c type : semantic output type, defaulting to \c "event" when omitted
 *
 * The returned vector preserves the order found in the option node, which is useful when the
 * caller wants output instantiation or reporting to follow user configuration order.
 *
 * \param gopts Parsed options container supplying the \c "gstreamer" node.
 * \return Vector of configured streamer definitions.
 */
std::vector<GStreamerDefinition> getGStreamerDefinition(const std::shared_ptr<GOptions>& gopts);

/**
 * \ingroup gstreamer_options_api
 * \brief Define the options contributed by the gstreamer module.
 *
 * The returned GOptions object contains:
 * - gstreamer-specific options such as \c ebuffer and \c -gstreamer
 * - the option definitions contributed by dependent modules currently aggregated here
 *
 * This function is typically used by applications and examples as the module entry point for
 * command-line configuration.
 *
 * \return Fully populated option definition object for the module.
 */
GOptions defineOptions();

}