#pragma once

// geant4
#include "G4UserEventAction.hh"

// gemc
#include "glogger.h"
#include "../run/gRunAction.h"

constexpr const char* EVENTACTION_LOGGER = "geventaction";

namespace geventaction {
inline GOptions defineOptions() { return GOptions(EVENTACTION_LOGGER); }
}

// Local thread classes
class GEventAction : public G4UserEventAction {
public:
	GEventAction(const std::shared_ptr<GOptions>& gopt, GRunAction* run_a);
	~GEventAction() override;

	void BeginOfEventAction(const G4Event* event) override;
	void EndOfEventAction(const G4Event* event) override;

private:
	std::shared_ptr<GLogger> log;
	std::shared_ptr<GOptions> goptions; // keeping the goption pointer to construct gdata
	GRunAction* run_action; // non-owning, valid for the thread lifetime

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