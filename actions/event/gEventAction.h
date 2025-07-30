#pragma once

// geant4
#include "G4UserEventAction.hh"

// gemc
#include "glogger.h"

constexpr const char* EVENTACTION_LOGGER = "EventAction";

namespace geventaction {
inline GOptions defineOptions() { return GOptions(EVENTACTION_LOGGER); }
}

// Local thread classes
class GEventAction : public G4UserEventAction {
public:
	GEventAction(std::shared_ptr<GOptions> gopt);
	~GEventAction() override;

	void BeginOfEventAction(const G4Event* event) override;
	void EndOfEventAction(const G4Event* event) override;

private:
	std::shared_ptr<GLogger> log;

};
