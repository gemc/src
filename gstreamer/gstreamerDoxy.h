#pragma once

/**
 * \mainpage gstreamer module
 *
 * \section gstreamer_overview Overview
 * The gstreamer module provides a plugin-based output layer that consumes event or frame data
 * produced by GEMC and serializes it to one or more output formats (e.g. ROOT, ASCII, CSV, JLABSRO, JSON).
 *
 * A streamer is configured via the `-gstreamer` option (see gstreamer options help)
 * and instantiated at runtime by loading the corresponding plugin:
 *
 * - `format: root`   -> `gstreamer_root_plugin`
 * - `format: ascii`  -> `gstreamer_ascii_plugin`
 * - `format: csv`    -> `gstreamer_csv_plugin`
 * - `format: jlabsro`-> `gstreamer_jlabsro_plugin`
 * - `format: json`   -> `gstreamer_json_plugin`
 *
 * \section gstreamer_arch Architecture
 * The core concepts are:
 * - \ref GStreamer : abstract base class that implements buffering and the high-level publish flow.
 * - GStreamerDefinition : lightweight description of an output (format, base filename, type, thread id).
 * - Factory plugins: shared libraries exposing an extern "C" factory symbol named \c GStreamerFactory
 *   returning a \ref GStreamer instance.
 *
 * The \ref GStreamer base class owns an event buffer and calls protected virtual hooks in a fixed order
 * when flushing buffered events. Concrete plugins override only the hooks they support.
 *
 * \section gstreamer_options Options
 * The module contributes options through gstreamer::defineOptions().
 * Notable options include:
 * - \c ebuffer : number of events kept in memory per streamer before flushing to the output medium.
 * - \c -gstreamer : list of output definitions (format, filename, type).
 *
 * \section gstreamer_verbosity Verbosity and logging
 * Classes in this module use a logger derived from the GEMC logging infrastructure (GLogger / glogger).
 * The typical meaning of verbosity levels is:
 * - level 0: high-level, user-facing progress (file opened/closed, run-level milestones).
 * - level 1: additional operational information (configuration details, non-fatal warnings).
 * - level 2: detailed per-event or per-action information (buffer flushes, per-detector publish steps).
 * Debug output (\c log->debug(...)) prints developer-oriented traces (function entry/exit,
 * internal state transitions, and diagnostics useful while debugging).
 *
 * \section gstreamer_examples Examples
 * The examples below are part of this module and are meant to be compiled and executed as
 * standalone demonstrations.
 *
 * \subsection gstreamer_example_multithread examples/gstreamer_example.cc
 * Demonstrates publishing synthetic event data from multiple worker threads. Each thread creates
 * its own streamer map, opens connections, publishes events, and closes outputs at the end.
 */
