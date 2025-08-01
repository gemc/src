#pragma once

// geant4
#include "G4Run.hh"

// gemc
#include "goptions.h"
#include "gdynamicdigitization.h"

constexpr const char* GRUN_LOGGER = "grun";

using GHitsCollection = G4THitsCollection<GHit>;

namespace grun {
inline GOptions defineOptions() { return GOptions(GRUN_LOGGER); }
}

// In Geant4 a run consists of a sequence of events and starts with BeamOn() method of G4RunManager.
// A Geant4 run is represented by a G4Run class object.
// G4GRun is created by G4MTRunManager, initialized in gemc.cc
// The pointers to the digitization and streamer are kept to be passed along RecordEvent (digitized event) and Merge (streams it out)
class GRun : public G4Run {
public:
	GRun(std::shared_ptr<GOptions> gopts, std::shared_ptr<gdynamicdigitization::dRoutinesMap> digi_map);
	~GRun() override;

private:
	// digitization map, populated in ConstructSDandField
	std::shared_ptr<gdynamicdigitization::dRoutinesMap> digitization_routines_map;
	std::shared_ptr<GLogger>  log;

};

// RecordEvent is called at the end of every event
// Method to be overwritten by the user for recording events in this (thread-local) run.
// The observables defined in each run should be filled here with the information from the hits
// void RecordEvent(const G4Event*) override;
// Method to be overwritten by the user for merging local Run objects to the global Run object
// void Merge(const G4Run*) override;
// this could be in merge: vector of events data in the run (local thread, merged in GRun::Merge in the global thread)
// vector<GEventDataCollection*> runData;
// inline vector<GEventDataCollection*> getRunData() const { return runData; }
