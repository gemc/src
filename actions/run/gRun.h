#pragma once

// geant4
#include "G4Run.hh"


// glibrary
#include "goptions.h"
#include "gdynamicdigitization.h"
#include "event/gEventDataCollection.h"

constexpr const char* GRUN_LOGGER = "grun";


// In Geant4 a run consists of a sequence of events and starts with BeamOn() method of G4RunManager.
// A Geant4 run is represented by a G4Run class object.
// G4GRun is created by G4MTRunManager, initialized in gemc.cc
// The pointers to the digitization and streamer are kept to be passed along RecordEvent (digitized event) and Merge (streams it out)
class GRun : public G4Run {
public:
	GRun(std::shared_ptr<GOptions> gopts, std::shared_ptr<gdynamicdigitization::dRoutinesMap> digi_map);
	~GRun() override;
	void RecordEvent(const G4Event*) override;
	//void Merge(const G4Run*) override;

private:

	// digitization map, populated in ConstructSDandField
	std::shared_ptr<gdynamicdigitization::dRoutinesMap> digitization_routines_map;

	std::shared_ptr<GLogger>  log;

	// vector of events data in the run (local thread, merged in GRun::Merge in the global thread)
	// we're only going to use this in
	// vector<GEventDataCollection*> runData;


public:
	// inline vector<GEventDataCollection*> getRunData() const { return runData; }

};
