/**
 * \mainpage GData Library
 *
 * \section gdata_intro Introduction
 *
 * The GData library provides flexible in-memory containers for simulation truth, digitized
 * observables, event-level aggregation, run-level aggregation, and frame-style integrated payloads.
 *
 * Overview:
 * - GTrueInfoData stores simulation-level quantities for one hit
 * - GDigitizedData stores electronics-level quantities for one hit
 * - GDataCollection groups those hit-side objects per detector
 * - GEventDataCollection groups detector data per event
 * - GRunDataCollection integrates detector data across many events
 * - GFrameDataCollection groups streaming-style payloads by frame
 *
 * The design emphasizes:
 * - schema flexibility through keyed maps rather than hard-coded layouts
 * - explicit ownership of inserted objects
 * - stable integration behavior for run-style accumulation
 *
 * \section gdata_ownership Ownership and lifecycle
 *
 * The module follows a layered ownership model:
 *
 * Event-level lifecycle:
 * - GTrueInfoData and GDigitizedData objects are usually created per hit
 * - ownership is transferred into GDataCollection through \c std::unique_ptr
 * - detector collections are then owned by GEventDataCollection
 *
 * Run-level lifecycle:
 * - GRunDataCollection owns one GRunHeader and one detector-summary map
 * - detector-local accumulation is handled by GDataCollection
 * - the first stored entry typically becomes the accumulator for integrated data
 *
 * Frame-level lifecycle:
 * - GFrameDataCollection currently owns its frame header and payload objects through raw pointers
 * - callers must not delete those objects after ownership has been transferred
 *
 * Design note:
 * - event containers represent many per-hit entries
 * - integrated containers typically represent one accumulated entry per detector
 *
 * \section gdata_architecture Architecture
 *
 * \subsection gdata_arch_layers Design notes
 *
 * The module is organized around progressively larger aggregation scopes:
 *
 * Hit scope:
 * - GTrueInfoData
 * - GDigitizedData
 *
 * Detector scope:
 * - GDataCollection
 *
 * Event scope:
 * - GEventHeader
 * - GEventDataCollection
 *
 * Run scope:
 * - GRunHeader
 * - GRunDataCollection
 *
 * Frame scope:
 * - GFrameHeader
 * - GIntegralPayload
 * - GFrameDataCollection
 *
 * Important design notes:
 * - keyed maps allow detector-specific schemas without changing the core types
 * - integration is additive rather than statistical
 * - streaming-readout keys can be separated from non-SRO observables for output and validation
 * - current run integration focuses on digitized content
 *
 * \section gdata_options Available Options and their usage
 *
 * This module exposes logger-domain option bundles through helper functions:
 * - \c gtrue_data::defineOptions()
 * - \c gdigi_data::defineOptions()
 * - \c geventheader::defineOptions()
 * - \c gevent_data::defineOptions()
 * - \c grun_header::defineOptions()
 * - \c grun_data::defineOptions()
 *
 * Typical usage:
 * \code
 * auto gopts = std::make_shared<GOptions>(argc, argv, gevent_data::defineOptions());
 * \endcode
 *
 * Aggregated bundles:
 * - \c gevent_data::defineOptions() includes event-header, true-data, digitized-data, and touchable options
 * - \c grun_data::defineOptions() includes event and run related options needed by the run example
 *
 * Usage pattern:
 * - use the smallest bundle that matches the module you are instantiating
 * - use aggregated bundles in examples or applications that combine several classes
 *
 * \section gdata_verbosity Module verbosity
 *
 * Several classes in this module derive from the common base and log through domain-specific
 * logger names such as:
 * - \c true_data
 * - \c digitized_data
 * - \c event_header
 * - \c gevent_data
 * - \c run_header
 * - \c grun_data
 *
 * Expected info-level convention in this module:
 * - level 0 prints high-level summaries, major processing milestones, and example output headlines
 * - level 1 prints medium-detail bookkeeping such as header summaries and integration progress
 * - level 2 prints fine-grained tracing such as variable insertion, accumulation, and filtered access
 *
 * Debug output is intended for developer diagnostics and lifecycle tracing, including constructor,
 * destructor, and low-level state-management activity.
 *
 * \section gdata_examples_overview Available examples
 *
 * The examples directory contains small standalone programs that illustrate the intended ownership
 * and aggregation patterns in this module.
 *
 * \subsection event_example Event example
 * Summary:
 * - demonstrates event-level construction of detector data
 * - shows how truth and digitized hit-side objects are inserted and inspected
 *
 * Link:
 * - \ref event_example
 *
 * Example snippet:
 * \code
 * auto edc = GEventDataCollection::create(gopts);
 * edc->addDetectorDigitizedData("ctof", GDigitizedData::create(gopts));
 * edc->addDetectorTrueInfoData("ctof", GTrueInfoData::create(gopts));
 * \endcode
 *
 * \subsection run_example Run example
 * Summary:
 * - demonstrates accumulation of many event collections into one run summary
 * - compares integrated results against reference sums computed directly from events
 *
 * Link:
 * - \ref run_example
 *
 * Example snippet:
 * \code
 * auto runData = std::make_shared<GRunDataCollection>(gopts, std::move(runHeader));
 * runData->collect_event_data_collection(edc);
 * \endcode
 *
 * \subsection gframe_example Frame example
 * Summary:
 * - demonstrates frame/time-window grouping of packed readout payloads
 * - shows the relationship between frame headers and integral payload storage
 *
 * Link:
 * - \ref gframe_example
 *
 * Example snippet:
 * \code
 * auto frameHeader = new GFrameHeader(frameID, frameDuration, log);
 * auto frameData   = new GFrameDataCollection(frameHeader, log);
 * frameData->addIntegralPayload({1, 2, 3, 100, 50});
 * \endcode
 *
 * \author \n &copy; Maurizio Ungaro
 * \author e-mail: ungaro@jlab.org
 */