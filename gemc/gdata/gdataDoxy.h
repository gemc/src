/**
 * \file gdataDoxy.h
 * \brief Main Doxygen page for the GData module.
 */

/**
 * \defgroup gdata_module_overview GData module overview
 * \brief Top-level documentation entry point for the GData module.
 *
 * \details
 * This topic groups the major concepts implemented by the module:
 * - truth-side hit data
 * - digitized hit data
 * - detector-local aggregation
 * - event-level aggregation
 * - run-level aggregation
 * - frame-level aggregation
 */

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
 * - explicit ownership transfer of inserted objects
 * - stable integration behavior for run-style accumulation
 * - separation of conventional streaming-readout keys from detector-specific observables
 *
 * \section gdata_ownership Ownership and lifecycle
 *
 * The module follows a layered ownership model.
 *
 * Hit-level lifecycle:
 * - GTrueInfoData and GDigitizedData objects are usually created per hit
 * - each object copies the originating hit identity so it remains valid after the source hit expires
 *
 * Detector-level lifecycle:
 * - ownership of hit objects is transferred into GDataCollection through \c std::unique_ptr
 * - in event mode, detector vectors grow by appending one object per hit
 * - in integrated mode, the first entry typically becomes the detector-local accumulator
 *
 * Event-level lifecycle:
 * - GEventDataCollection owns one GEventHeader
 * - GEventDataCollection owns the per-detector map of GDataCollection objects
 *
 * Run-level lifecycle:
 * - GRunDataCollection owns one GRunHeader and one detector-summary map
 * - detector-local accumulation is delegated to GDataCollection
 *
 * Frame-level lifecycle:
 * - GFrameDataCollection currently owns its frame header and payload objects through raw pointers
 * - callers must not delete those objects after ownership has been transferred
 *
 * \section gdata_architecture Architecture
 *
 * \subsection gdata_arch_layers Design notes
 *
 * The module is organized around progressively larger aggregation scopes.
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
 * - digitized filtering separates conventional SRO fields from non-SRO content
 * - event containers model many hit entries
 * - run containers usually model one integrated entry per detector
 * - frame containers model time-window grouping rather than event grouping
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
 * If options inherited from GOptions::defineOptions() also apply in the surrounding application,
 * they remain valid for this module because all of these helpers build or aggregate GOptions
 * subtrees that are later combined by the application.
 *
 * Typical usage:
 * \code
 * auto gopts = std::make_shared<GOptions>(argc, argv, gevent_data::defineOptions());
 * \endcode
 *
 * Aggregated bundles:
 * - \c gevent_data::defineOptions() includes event-header, true-data, digitized-data, and touchable options
 * - \c grun_data::defineOptions() includes event, run, digitized, and touchable-related options needed by the run example
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
 * The examples directory contains small standalone programs illustrating the intended ownership
 * and aggregation patterns in this module.
 *
 * \subsection event_example_summary Event example
 * Summary:
 * - demonstrates event-level construction of detector data
 * - shows how truth and digitized hit-side objects are inserted and inspected
 *
 * Link to example mainpage:
 * - \ref event_example
 *
 * Example snippet:
 * \code
 * auto edc = GEventDataCollection::create(gopts);
 * edc->addDetectorDigitizedData("ctof", GDigitizedData::create(gopts));
 * edc->addDetectorTrueInfoData("ctof", GTrueInfoData::create(gopts));
 * \endcode
 *
 * \subsection run_example_summary Run example
 * Summary:
 * - demonstrates accumulation of many event collections into one run summary
 * - compares integrated results against reference sums computed directly from events
 *
 * Link to example mainpage:
 * - \ref run_example
 *
 * Example snippet:
 * \code
 * auto runData = std::make_shared<GRunDataCollection>(gopts, std::move(runHeader));
 * runData->collect_event_data_collection(edc);
 * \endcode
 *
 * \subsection gframe_example_summary Frame example
 * Summary:
 * - demonstrates frame/time-window grouping of packed readout payloads
 * - shows the relationship between frame headers and integral payload storage
 *
 * Link to example mainpage:
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