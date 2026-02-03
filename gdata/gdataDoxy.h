/**
 * \mainpage GData Library
 *
 * \section intro_sec Introduction
 * The GData library provides classes and utilities to accumulate simulated “true” information
 * and digitized observables into custom, transportable objects.
 *
 * The design emphasizes:
 * - **schema flexibility** (string-keyed maps instead of fixed bank layouts)
 * - **clear ownership** (event-level objects owned via \c std::unique_ptr)
 * - **deterministic integration** (run-level accumulation semantics)
 *
 * Typical consumers include:
 * - output backends that serialize hit data into formats such as ROOT/HIPO/JSON/etc.
 * - analysis and QA tools that want a stable in-memory representation of event/run content
 * - plugin-based digitization systems where each detector defines its own variables
 *
 * \section model_sec Data model
 *
 * \subsection truth_sec True hit information (simulation-level)
 * - \ref GTrueInfoData stores truth observables as maps of:
 *   - doubles (energy deposition, positions, time, etc.)
 *   - strings (metadata/provenance)
 *
 * Truth objects also carry a hit identity vector copied from \ref GHit, accessible via
 * \ref GTrueInfoData::getIdentityString "getIdentityString()".
 *
 * \subsection digi_sec Digitized information (electronics-level)
 * - \ref GDigitizedData stores digitized observables as maps of:
 *   - ints (indices, integerized readout, and some SRO keys such as crate/slot/channel/timeAtElectronics)
 *   - doubles (ADC-like values, calibrated timing/energy, etc.)
 *
 * It supports optional filtering of streaming readout (SRO) keys:
 * - crate / slot / channel / timeAtElectronics / chargeAtElectronics
 *
 * via \ref GDigitizedData::getIntObservablesMap "getIntObservablesMap()"
 * and \ref GDigitizedData::getDblObservablesMap "getDblObservablesMap()".
 *
 * \subsection collection_sec Collections
 * - \ref GDataCollection groups true/digitized objects for a single sensitive detector.
 * - \ref GEventDataCollection aggregates detector data for one event (keyed by SD name).
 * - \ref GRunDataCollection integrates many events into a run summary (accumulation semantics).
 * - \ref GFrameDataCollection aggregates readout payloads per frame/time-window.
 *
 * \section verbosity_sec Module verbosity
 * Most GData classes emit messages through the common logging system (\c GLogger) and can be
 * configured by logger “domains” (for example: \c "true_data", \c "digitized_data", \c "gevent_data",
 * \c "grun_data"). These domains are typically enabled and tuned via the application options layer
 * (\c GOptions) so you can increase detail for one component without flooding the whole application.
 *
 * The \c info(level, ...) calls follow a coarse convention across this module:
 * - **info level 0**: high-level summaries intended for normal runs (major steps, per-event or per-run headlines).
 * - **info level 1**: moderately detailed progress and bookkeeping (per-detector summaries, integration milestones).
 * - **info level 2**: fine-grained tracing useful for debugging logic and data flow
 *   (per-hit actions, per-variable insertion/accumulation, filtering decisions).
 *
 * \c debug(...) messages are intended for developer diagnostics and lifecycle tracing. In this module they
 * typically cover constructor/destructor activity, low-level state transitions, and “why did we do this?”
 * decisions (for example: creation of accumulators, internal container management, and verbose frame/payload traces).
 *
 * \section examples_sec Examples
 * The repository includes small, buildable examples (see \c examples/) intended to demonstrate the
 * intended ownership patterns and integration semantics. Each example is a standalone program and can
 * be used as a reference when integrating GData into an application or backend.
 *
 * \subsection examples_event_subsec \ref gdata_event_example "Event data collection example"
 * Demonstrates **event-level ownership**: creating a \ref GEventDataCollection, adding hits for one or more
 * detectors, printing identity strings, and inspecting truth/digitized observables (including SRO filtering).
 *
 * \subsection examples_run_subsec \ref gdata_run_example "Run data integration example"
 * Demonstrates **run-level accumulation**: integrating many \ref GEventDataCollection objects into a single
 * \ref GRunDataCollection and validating that integrated sums match a reference scan of the event content.
 *
 * \subsection examples_frame_subsec \ref gdata_frame_example "Frame data example"
 * Demonstrates **frame/time-window grouping**: building a \ref GFrameDataCollection from a \ref GFrameHeader and
 * adding packed \ref GIntegralPayload samples (crate/slot/channel/charge/time) as a minimal streaming-style model.
 *
 * \section notes_sec Ownership and integration semantics
 *
 * \subsection own_sec Ownership
 * - Event-level collections typically \b own per-hit objects using \c std::unique_ptr.
 * - Callers transfer ownership when inserting hits into event containers.
 *
 * \subsection integrate_sec Integration
 * Run-level integration typically stores a single entry per detector and calls:
 * - \ref GTrueInfoData::accumulateVariable "accumulateVariable()" (truth doubles)
 * - \ref GDigitizedData::accumulateVariable "accumulateVariable()" (digitized non-SRO scalars)
 *
 * \subsection normalize_sec Normalization
 * Accumulated values are sums by construction. If you need averages or rates:
 * - divide by number of hits or events
 * - for streaming/frame data, divide by frame duration or time window
 *
 * \author \n &copy; Maurizio Ungaro
 * \author e-mail: ungaro@jlab.org
 */
