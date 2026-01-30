/**
 * \mainpage GData Library
 *
 * \tableofcontents
 *
 * \section intro_sec Introduction
 * The GData library provides classes and utilities to accumulate simulated “true” information
 * and digitized observables into custom, transportable objects.
 *
 * The key design goal is to let detector/digitization plugins define arbitrary observables
 * using string keys, while providing standard containers to:
 * - collect per-event/per-hit data
 * - integrate data across events (run-level accumulation)
 * - group readout-like payloads into time windows (frame-level)
 *
 * \section model_sec Data model
 * \subsection truth_sec True hit information (simulation-level)
 * - \ref GTrueInfoData stores truth observables as maps of:
 *   - doubles (energy deposition, positions, time, etc.)
 *   - strings (metadata/provenance)
 *
 * \subsection digi_sec Digitized information (electronics-level)
 * - \ref GDigitizedData stores digitized observables as maps of:
 *   - ints (indices, integerized readout)
 *   - doubles (ADC-like values, calibrated timing/energy)
 * - It supports optional filtering of streaming readout (SRO) keys:
 *   crate/slot/channel/timeAtElectronics/chargeAtElectronics.
 *
 * \subsection collection_sec Collections
 * - \ref GDataCollection groups true/digitized objects for a single sensitive detector.
 * - \ref GEventDataCollection aggregates detector data for one event (keyed by SD name).
 * - \ref GRunDataCollection integrates many events into a run summary (accumulation semantics).
 * - \ref GFrameDataCollection aggregates readout payloads per frame/time-window.
 *
 * \section examples_sec Examples
 * The following example pages are built from the files in \c examples/:
 * - \ref gdata_event_example
 * - \ref gdata_run_example
 * - \ref gdata_frame_example
 *
 * \section notes_sec Ownership and integration semantics
 * \subsection own_sec Ownership
 * - Event-level collections typically \b own per-hit objects using \c std::unique_ptr.
 * - Run-level integration typically stores a single entry per detector and calls
 *   \c accumulateVariable() to build sums across events.
 *
 * \subsection normalize_sec Normalization
 * Accumulated values are sums by construction. If you need averages or rates,
 * normalize in the consumer (divide by number of hits/events, frame duration, etc.).
 *
 * \author \n &copy; Maurizio Ungaro
 * \author e-mail: ungaro@jlab.org
 */
