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
 * \ref GTrueInfoData::getIdentityString.
 *
 * \subsection digi_sec Digitized information (electronics-level)
 * - \ref GDigitizedData stores digitized observables as maps of:
 *   - ints (indices, integerized readout, and some SRO keys such as crate/slot/channel/timeAtElectronics)
 *   - doubles (ADC-like values, calibrated timing/energy, etc.)
 *
 * It supports optional filtering of streaming readout (SRO) keys:
 * - crate / slot / channel / timeAtElectronics / chargeAtElectronics
 *
 * via \ref GDigitizedData::getIntObservablesMap and \ref GDigitizedData::getDblObservablesMap.
 *
 * \subsection collection_sec Collections
 * - \ref GDataCollection groups true/digitized objects for a single sensitive detector.
 * - \ref GEventDataCollection aggregates detector data for one event (keyed by SD name).
 * - \ref GRunDataCollection integrates many events into a run summary (accumulation semantics).
 * - \ref GFrameDataCollection aggregates readout payloads per frame/time-window.
 *
 * \section examples_sec Examples
 * Example pages built from files in \c examples/:
 * - \ref gdata_event_example : event-level ownership, hit insertion, inspection.
 * - \ref gdata_run_example   : run-level accumulation and validation strategy.
 * - \ref gdata_frame_example : frame/time-window payload grouping (streaming-style).
 *
 * \section notes_sec Ownership and integration semantics
 *
 * \subsection own_sec Ownership
 * - Event-level collections typically \b own per-hit objects using \c std::unique_ptr.
 * - Callers transfer ownership when inserting hits into event containers.
 *
 * \subsection integrate_sec Integration
 * Run-level integration typically stores a single entry per detector and calls:
 * - \ref GTrueInfoData::accumulateVariable (truth doubles)
 * - \ref GDigitizedData::accumulateVariable (digitized non-SRO scalars)
 *
 * \subsection normalize_sec Normalization
 * Accumulated values are sums by construction. If you need averages or rates:
 * - divide by number of hits or events
 * - for streaming/frame data, divide by frame duration or time window
 *
 * \author \n &copy; Maurizio Ungaro
 * \author e-mail: ungaro@jlab.org
 */
