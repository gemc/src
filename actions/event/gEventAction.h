#pragma once

// geant4
#include "G4UserEventAction.hh"

// gemc
#include "gbase.h"
#include "../run/gRunAction.h"

/**
 * @file gEventAction.h
 * @brief Declares GEventAction, responsible for per-event lifecycle hooks and event publication.
 *
 * @ingroup gactions_module
 */

constexpr const char* EVENTACTION_LOGGER = "geventaction";

/**
 * @brief Namespace collecting helpers for the event action.
 *
 * @ingroup gactions_module
 */
namespace geventaction {
/**
 * @brief Returns the options associated with the event action.
 *
 * @return A GOptions instance for the event action logger scope.
 */
inline GOptions defineOptions() { return GOptions(EVENTACTION_LOGGER); }
} // namespace geventaction


/**
 * @class GEventAction
 * @brief Handles event begin/end callbacks and triggers digitization + streaming.
 *
 * Responsibilities:
 * - At event begin: optionally log event/thread identification.
 * - At event end:
 *   - Retrieve the hit collections for the event.
 *   - For each hit collection:
 *     - Resolve the digitization routine associated with the collection name.
 *     - Convert hits into digitized data and true-information data.
 *     - Add these products to the event data container.
 *   - Publish the completed event data to all configured streamers for the thread.
 *
 * Ownership:
 * - The run_action pointer is non-owning; it is expected to remain valid for the
 *   lifetime of the thread actions (it is created and registered by GAction).
 *
 * @ingroup gactions_module
 */
class GEventAction : public GBase<GEventAction>, public G4UserEventAction {
public:
	/**
	 * @brief Constructs the event action.
	 *
	 * @param gopt Shared configuration used to construct event data containers and control logging.
	 * @param run_a Non-owning pointer to the thread's GRunAction instance, used to access
	 *              digitization routines and the streamer map.
	 */
	GEventAction(const std::shared_ptr<GOptions>& gopt, GRunAction* run_a);

	/**
	 * @brief Called by Geant4 at the beginning of an event.
	 *
	 * Typical usage in this module is logging and lightweight per-event bookkeeping.
	 *
	 * @param event The Geant4 event descriptor.
	 */
	void BeginOfEventAction(const G4Event* event) override;

	/**
	 * @brief Called by Geant4 at the end of an event.
	 *
	 * This method performs the event-level workflow:
	 * - Collect hit collections.
	 * - Digitize hits and collect truth information.
	 * - Publish the resulting event data to streamers.
	 *
	 * @param event The Geant4 event descriptor.
	 */
	void EndOfEventAction(const G4Event* event) override;

private:
	/**
	 * @brief Shared configuration used for constructing event products and controlling logging.
	 */
	std::shared_ptr<GOptions> goptions;

	/**
	 * @brief Non-owning pointer to the thread's run action.
	 *
	 * This is used to access:
	 * - The digitization routines map (collection name -> routine).
	 * - The per-thread streamer map.
	 */
	GRunAction* run_action;
};


// looping over output factories
// for (auto [factoryName, streamerFactory] : *gstreamerFactoryMap) {
// 	if (streamerFactory->getStreamType() == "event") {
// 		logSummary("Writing event data using streamer factory >" + factoryName + "<");
// 		map<string, bool> streamReport = streamerFactory->publishEventRunData(goptions, theRun->getRunData());
//
// 		for (auto [reportName, result] : streamReport) {
// 			string resultString = result ? " success" : " failure";
// 			logSummary("Factory <" + factoryName + "> " + reportName + resultString);
// 		}
// 	}
// }
