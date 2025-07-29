#pragma once

// geant4
#include "G4UserRunAction.hh"


// gemc
#include "glogger.h"
#include "gdynamicdigitization.h"
#include "gstreamer.h"
#include "frame/gFrameDataCollection.h"


constexpr const char* GRUNACTION_LOGGER = "grunaction";

class GRunAction : public G4UserRunAction {
public:
	// constructor and destructor
	GRunAction(std::shared_ptr<GOptions> gopts, std::shared_ptr<gdynamicdigitization::dRoutinesMap> digi_map);
	~GRunAction() override;

private:

	// virtual method from G4UserRunAction.
	G4Run* GenerateRun() override;
	void   BeginOfRunAction(const G4Run*) override;
	void   EndOfRunAction(const G4Run*) override;

	std::shared_ptr<GOptions> goptions; // keeping the goption pointer to construct the other actions
	std::shared_ptr<GLogger>  log;

	// digitization map, populated in ConstructSDandField
	std::shared_ptr<gdynamicdigitization::dRoutinesMap> digitization_routines_map;

	// streamer map, instantiated in threads (Build)
	std::shared_ptr<const gstreamer::gstreamersMap> gstreamer_map;


	// vector of frame data in the run (local thread, merged in GRun::Merge in the global thread)
//	std::vector<GFrameDataCollection*> frameRunData;


	//
	// double frameDuration    = UNINITIALIZEDNUMBERQUANTITY; // frame length in nanoseconds
	// double eventDuration    = UNINITIALIZEDNUMBERQUANTITY; // event duration in nanoseconds
	// int    eventIndex       = 0;                           // added to the absolute event number, increases with each run
	// int    lastFrameCreated = 0;                           // keeping track of the last frame created
	//
	// // determine the frame ID based on event number, eventDuration, frameDuration
	// // TODO: this should be in gdata?
	// int  eventFrameIndex(int eventNumber, double timeAtElectronics);
	// bool findFrameID(int fid);
	//
	// // decide whether to write or not to stream the frame based on event number, eventDuration, frameDuration and number of threads
	// // streaming the frame also deletes it from frameRunData
	// // TODO: this should be in gdata?
	// bool shouldWriteFrameID(int eventNumber, long int frameID);
	//
	// // TODO: this should be in gdata?
	// std::vector<int> formPayload(GDigitizedData* digitizedData);

};
