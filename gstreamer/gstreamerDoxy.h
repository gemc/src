#pragma once

/**
 * \file gstreamerDoxy.h
 * \brief Main Doxygen entry point for the gstreamer module.
 */

/**
 * \defgroup gstreamer_module gstreamer module
 * \brief Plugin-based output infrastructure for event, run, and frame data serialization.
 *
 * The gstreamer module provides the abstract streaming interface, option definitions,
 * concrete output plugins, and helper utilities used to publish GEMC data products to
 * external representations such as text, CSV, JSON, ROOT, and binary frame streams.
 */

/**
 * \defgroup gstreamer_options_api gstreamer option handling
 * \ingroup gstreamer_module
 * \brief Types and functions used to define and parse module configuration options.
 */

/**
 * \defgroup gstreamer_core_api gstreamer core streaming API
 * \ingroup gstreamer_module
 * \brief Core abstractions and helper functions used by all streamer plugins.
 */

/**
 * \defgroup gstreamer_plugin_ascii_api ASCII streamer plugin
 * \ingroup gstreamer_module
 * \brief Human-readable text streamer plugin for events, runs, and frames.
 */

/**
 * \defgroup gstreamer_plugin_csv_api CSV streamer plugin
 * \ingroup gstreamer_module
 * \brief CSV streamer plugin that flattens detector hit content into per-hit rows.
 */

/**
 * \defgroup gstreamer_plugin_json_api JSON streamer plugin
 * \ingroup gstreamer_module
 * \brief JSON streamer plugin for structured event and frame serialization.
 */

/**
 * \defgroup gstreamer_plugin_root_api ROOT streamer plugin
 * \ingroup gstreamer_module
 * \brief ROOT streamer plugin writing event and run content into \c TTree objects.
 */

/**
 * \defgroup gstreamer_plugin_jlabsro_api JLAB SRO streamer plugin
 * \ingroup gstreamer_module
 * \brief Binary frame streamer plugin producing packed JLAB SRO records.
 */

/**
 * \defgroup gstreamer_examples_api gstreamer examples
 * \ingroup gstreamer_module
 * \brief Example programs demonstrating how to configure and use the module.
 */

/**
 * \mainpage gstreamer module
 *
 * \section gstreamer_intro Introduction
 * The gstreamer module is the output layer responsible for taking in-memory GEMC data collections
 * and serializing them through a plugin architecture. The module supports multiple output backends,
 * allowing the same event or frame production flow to be directed to different storage formats
 * without changing the event-building code.
 *
 * In practice, a user configures one or more outputs through the \c -gstreamer option, and the
 * module loads the corresponding plugin libraries at runtime. Each plugin then receives a
 * high-level publish sequence from the common \ref GStreamer interface.
 *
 * Supported built-in format tokens currently include:
 * - \c root
 * - \c ascii
 * - \c csv
 * - \c jlabsro
 * - \c json
 *
 * These map to plugin names using the standard naming convention:
 * - \c gstreamer_root_plugin
 * - \c gstreamer_ascii_plugin
 * - \c gstreamer_csv_plugin
 * - \c gstreamer_jlabsro_plugin
 * - \c gstreamer_json_plugin
 *
 * \section gstreamer_ownership Ownership and lifecycle
 * The module is designed around explicit ownership boundaries:
 * - Configuration is provided through GOptions.
 * - Streamer definitions are represented by GStreamerDefinition values.
 * - Concrete streamer instances are created dynamically by the plugin manager.
 * - In multithreaded usage, each worker thread typically owns its own streamer map and therefore
 *   its own plugin instances.
 *
 * Event data ownership remains outside the streamer. The base streamer only stores
 * \c std::shared_ptr<GEventDataCollection> objects in its buffer so that all event-owned hit data
 * remain valid until the flush is complete. Concrete plugins receive read-only views during
 * publication and do not take ownership of the source event content.
 *
 * The normal lifecycle is:
 * - define options
 * - parse user configuration
 * - create per-thread streamer instances
 * - call \ref GStreamer::define_gstreamer "define_gstreamer()"
 * - call \ref GStreamer::set_loggers "set_loggers()"
 * - open connections
 * - publish run, event, or frame data
 * - close connections
 *
 * \section gstreamer_arch Architecture
 * The architecture is intentionally layered:
 *
 * \subsection gstreamer_arch_core Core abstractions
 * - \ref GStreamer : abstract base class implementing buffering and the publish flow.
 * - \ref GStreamerDefinition : value type describing one configured output.
 * - gstreamer namespace helpers : parse option nodes and instantiate per-thread streamer maps.
 *
 * \subsection gstreamer_arch_plugins Plugin layer
 * Each concrete plugin derives from \ref GStreamer and overrides only the hooks relevant to its
 * output model. For event publication, the base class drives a fixed sequence:
 * - \ref GStreamer::startEvent "startEvent()"
 * - \ref GStreamer::publishEventHeader "publishEventHeader()"
 * - \ref GStreamer::publishEventTrueInfoData "publishEventTrueInfoData()"
 * - \ref GStreamer::publishEventDigitizedData "publishEventDigitizedData()"
 * - \ref GStreamer::endEvent "endEvent()"
 *
 * Similar hook sequences exist for run data and frame streams.
 *
 * \subsection gstreamer_arch_design Design notes
 * Design choices in this module include:
 * - buffering is centralized in the base class so plugins stay focused on serialization
 * - format discovery is runtime-based through plugin naming conventions
 * - streamer instances are expected to be thread-local in normal operation
 * - frame streaming flushes pending event buffers first so event and frame outputs are not mixed
 *   unintentionally in the same stream
 *
 * \section gstreamer_options_section The list of Available Options and their usage
 * The module contributes the following user-facing options through
 * gstreamer::defineOptions() :
 *
 * \subsection gstreamer_options_primary Primary gstreamer options
 * - \c ebuffer
 *   - Meaning : number of events buffered in memory before automatic flush
 *   - Usage : tune batching behavior for throughput versus memory usage
 *   - Default : \c DEFAULT_GSTREAMER_BUFFER_FLUSH_LIMIT
 *
 * - \c -gstreamer
 *   - Meaning : list of output definitions
 *   - Required fields per entry :
 *     - \c format
 *     - \c filename
 *   - Optional field per entry :
 *     - \c type, defaulting to \c "event"
 *
 * Example:
 * \code
 * -gstreamer="[{format: root, filename: out_root}, {format: ascii, filename: out_txt, type: event}]"
 * \endcode
 *
 * \subsection gstreamer_options_inherited Options from dependent modules
 * The module also aggregates options defined by the gdynamicdigitization module through
 * gdynamicdigitization::defineOptions(). Those options apply when the example or application
 * simultaneously uses dynamic digitization and streaming.
 *
 * \section gstreamer_verbosity Module verbosity
 * Classes in this module log through the module logger category \c "gstreamer".
 *
 * Typical verbosity interpretation is:
 * - level 0 : high-level operational progress such as worker startup, output opening, and summary actions
 * - level 1 : plugin and connection management details such as file open and close messages
 * - level 2 : detailed publish flow, including event flush activity, per-detector output actions,
 *   and tree creation in plugins such as the ROOT backend
 *
 * Debug output is intended for developers. It prints function-level traces, lifecycle transitions,
 * and internal state information that help diagnose ordering, buffering, and plugin behavior.
 *
 * \section gstreamer_examples_section Examples
 * The following example is documented as part of this module:
 *
 * - \ref gstreamer_example "gstreamer_example" :
 *   multithreaded synthetic event publication using per-thread streamer maps
 *
 * Example snippet:
 * \code
 * auto gopts = std::make_shared<GOptions>(argc, argv, gstreamer::defineOptions());
 * auto gstreamer_map = gstreamer::gstreamersMapPtr(gopts, tid);
 *
 * for (const auto& [name, streamer] : *gstreamer_map) {
 *     streamer->openConnection();
 *     streamer->publishEventData(eventData);
 *     streamer->closeConnection();
 * }
 * \endcode
 */