// gemc
#include "gRunAction.h"
#include "gRun.h"
#include "../gactionConventions.h"

// geant4
#include "G4Threading.hh"
#include "G4MTRunManager.hh"


// Constructor for workers: stores shared services and logs thread identity.
GRunAction::GRunAction(std::shared_ptr<GOptions> gopt, std::shared_ptr<gdynamicdigitization::dRoutinesMap> digi_map) :
	GBase(gopt, GRUNACTION_LOGGER),
	goptions(gopt),
	digitization_routines_map(digi_map) {
	auto desc= std::to_string(G4Threading::G4GetThreadId());

	log->debug(CONSTRUCTOR, FUNCTION_NAME, desc);
}


// Executed after BeamOn(): create the thread-local run container.
G4Run* GRunAction::GenerateRun() {
	log->debug(NORMAL, FUNCTION_NAME);

	return new GRun(goptions, digitization_routines_map);
}

// Invoked at the beginning of BeamOn (before physics tables are computed).
void GRunAction::BeginOfRunAction(const G4Run* aRun) {

	int thread_id      = G4Threading::G4GetThreadId();
	int run            = aRun->GetRunID();
	int neventsThisRun = aRun->GetNumberOfEventToBeProcessed();

	// Lazily define the per-thread streamer map for worker threads only.
	if (!IsMaster() && gstreamer_map == nullptr) {
		log->info(0, " Defining gstreamers for thread id ", thread_id);
		gstreamer_map = gstreamer::gstreamersMapPtr(goptions, thread_id);
	}

	// (Re)-open streamer connections for this run on worker threads.
	// (Re)-open streamer connections for this run on worker threads.
	if (!IsMaster()) {
		if (gstreamer_map == nullptr) {
			log->error(1, FUNCTION_NAME, " gstreamer_map is null in thread ", thread_id, " - cannot open connections.");
		} else {
			for (auto& [name, gstreamer] : *gstreamer_map) {
				if (!gstreamer->openConnection()) {
					log->error(ERR_STREAMERMAP_NOT_EXISTING, "Failed to open connection for GStreamer ", name, " in thread ", thread_id);
				}
			}
		}
	}


	std::string what_am_i = IsMaster() ? "Master" : "Worker";

	log->info(2, FUNCTION_NAME, " ", what_am_i, " [", thread_id, "],  for run ", run, ", events to be processed: ", neventsThisRun);
}

// Invoked at the very end of the run processing: close worker-thread streamer connections.
void GRunAction::EndOfRunAction(const G4Run* aRun) {

	//	const GRun* theRun = static_cast<const GRun*>(aRun);
	int         thread_id = G4Threading::G4GetThreadId();
	int         run       = aRun->GetRunID();
	std::string what_am_i = IsMaster() ? "Master" : "Worker";

	if (!IsMaster()) {
		if (gstreamer_map == nullptr) {
			log->error(ERR_STREAMERMAP_NOT_EXISTING, FUNCTION_NAME, " gstreamer_map is null in thread ", thread_id, " - cannot close connections.");
		} else {
			for (const auto& [name, gstreamer] : *gstreamer_map) {
				log->info(2, FUNCTION_NAME, " ", what_am_i, " [", thread_id, "],  for run ", run,
						  " closing connection for gstreamer ", name);
				if (!gstreamer->closeConnection()) {
					log->error(1, "Failed to close connection for GStreamer ", name, " in thread ", thread_id);
				}
			}
		}
	}

}

// (Legacy/experimental streaming logic remains commented out below.)


// TODO: 2 more is too much we need some calculation here
// int nFramesToCreate = neventsThisRun * eventDuration / frameDuration + 2;

// if (stream) {
// 	if (frameStreamVerbosity >= GVERBOSITY_SUMMARY) {
// 		cout << SROLOGHEADER << " current nframes in the buffer: " << frameRunData.size() << ", new frames to create: " << nFramesToCreate;
// 		cout << ", last frame id created: " << lastFrameCreated << endl;
// 	}
//
// 	for (int f = lastFrameCreated; f < lastFrameCreated + nFramesToCreate; f++) {
// 		GFrameDataCollectionHeader* gframeHeader = new GFrameDataCollectionHeader(f + 1, frameDuration, verbosity);
// 		GFrameDataCollection*       frameData    = new GFrameDataCollection(gframeHeader, verbosity);
// 		frameRunData.push_back(frameData);
// 	}
//
// 	lastFrameCreated += nFramesToCreate;
// 	if (frameStreamVerbosity >= GVERBOSITY_SUMMARY) {
// 		cout << SROLOGHEADER << nFramesToCreate << " new frames, buffer size is now " << frameRunData.size();
// 		cout << ", last frame id created: " << lastFrameCreated << endl;
// 	}
// }


// looping over run data and filling frameRunData
// need to remember last event number here
// if (stream) {
// 	for (auto eventDataCollection : theRun->getRunData()) {
// 		int absoluteEventNumber = eventIndex + eventDataCollection->getEventNumber();
//
// 		// filling frameRunData with this eventDataCollection
// 		for (auto [detectorName, gdataCollection] : *eventDataCollection->getDataCollectionMap()) {
// 			for (auto hitDigitizedData : *gdataCollection->getDigitizedData()) {
// 				int timeAtelectronic = hitDigitizedData->getTimeAtElectronics();
// 				if (timeAtelectronic != TIMEATELECTRONICSNOTDEFINED) {
// 					int frameIndex = eventFrameIndex(absoluteEventNumber, timeAtelectronic);
// 					frameRunData[frameIndex]->addIntegralPayload(formPayload(hitDigitizedData), verbosity);
// 				}
// 			}
// 		}
// 	}
// }


// now flushing all frames past eventIndex

// if (stream) {
// 	// updating eventIndex
// 	eventIndex += neventsThisRun;
//
// 	for (auto [factoryName, streamerFactory] : *gstreamerFactoryMap) {
// 		if (streamerFactory->getStreamType() == "stream" && frameRunData.size() > 0) {
// 			// need to look for additional frame to flush
// 			int nFramesToFlush = nFramesToCreate - 2;
//
// 			if (frameStreamVerbosity >= GVERBOSITY_SUMMARY) { cout << SROLOGHEADER << "number of frames to flush: " << nFramesToFlush << endl; }
// 			for (auto fid = 0; fid < nFramesToFlush; fid++) {
// 				logSummary("Streaming frame id <" + to_string(frameRunData.front()->getFrameID()) + " using streamer factory >" + factoryName + "<");
// 				streamerFactory->publishFrameRunData(goptions, frameRunData.front());
// 				delete frameRunData.front();
// 				frameRunData.erase(frameRunData.begin());
// 			}
// 		}
// 	}
// }


// determine the frame ID based on event number, eventDuration, frameDuration and number of threads
// add frameData to frameRunData if it's not present
// int GRunAction::eventFrameIndex(int eventNumber, double timeAtElectronics) {
// 	int absoluteHitTime = eventNumber * eventDuration + timeAtElectronics;
// 	int frameID         = absoluteHitTime / frameDuration + 1;
// 	int frameIndex      = -1;
//
// 	// cout << "eventNumber: " << eventNumber << ", absoluteHitTime: " << absoluteHitTime << ", frameID: " << frameID << endl;
//
// 	for (size_t f = 0; f < frameRunData.size(); f++) { if (frameRunData[f]->getFrameID() == frameID) { frameIndex = (int)f; } }
// 	// cout << "eventNumber: " << eventNumber << ", absoluteHitTime: " << absoluteHitTime << ", frameIndex: " << frameIndex << endl;
//
// 	return frameIndex;
// }

// vector<int> GRunAction::formPayload(GDigitizedData* digitizedData) {
// 	vector<int> payload;
//
// 	int crate   = digitizedData->getIntObservable(CRATESTRINGID);
// 	int slot    = digitizedData->getIntObservable(SLOTSTRINGID);
// 	int channel = digitizedData->getIntObservable(CHANNELSTRINGID);
// 	int q       = digitizedData->getIntObservable(CHARGEATELECTRONICS);
// 	int time    = digitizedData->getIntObservable(TIMEATELECTRONICS);
//
// 	payload.push_back(crate);
// 	payload.push_back(slot);
// 	payload.push_back(channel);
// 	payload.push_back(q);
// 	payload.push_back(time);
//
// 	return payload;
// }
//
// bool GRunAction::findFrameID(int fid) {
// 	for (auto frame : frameRunData) { if (frame->getFrameID() == fid) { return true; } }
// 	return false;
// }
