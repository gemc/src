// gemc
#include "gRunAction.h"
#include "gRun.h"


// geant4
#include "G4Threading.hh"
#include "G4MTRunManager.hh"


// Constructor for workers
GRunAction::GRunAction(std::shared_ptr<GOptions> gopt, std::shared_ptr<gdynamicdigitization::dRoutinesMap> digi_map) :
	goptions(gopt),
	digitization_routines_map(digi_map) {

	auto desc =  "GRunAction " + std::to_string(G4Threading::G4GetThreadId()) ;

	log = std::make_shared<GLogger>(gopt, GRUNACTION_LOGGER, desc);

	log->debug(CONSTRUCTOR, FUNCTION_NAME);


	// frameDuration = 64000;
	// eventDuration = gutilities::getG4Number(goptions->getScalarString("eventTimeSize"));

	// stream = gopt->getSwitch("stream");
}


// Destructor
GRunAction::~GRunAction() { log->debug(DESTRUCTOR, FUNCTION_NAME); }

// TODO: this is not local?
// executed after BeamOn
G4Run* GRunAction::GenerateRun() {
	log->debug(NORMAL, FUNCTION_NAME);

	return new GRun(goptions, digitization_routines_map);
}

// invoked at the beginning of BeamOn (before physics tables are computed)
void GRunAction::BeginOfRunAction(const G4Run* aRun) {

	int thread_id = G4Threading::G4GetThreadId();
	int run       = aRun->GetRunID();

	if (IsMaster()) {
		log->info(2, FUNCTION_NAME, " for Master for run id", run, " in thread id: ", thread_id);
	}
	else {
		log->info(2, FUNCTION_NAME, " for Workers for run id", run, " in thread id: ", thread_id);

		// if not instantiated already, define gstreamer_map
		if (gstreamer_map == nullptr) { gstreamer_map = gstreamer::gstreamersMapPtr(goptions, thread_id); }
	}
}

// invoked at the very end of the run processing
void GRunAction::EndOfRunAction(const G4Run* aRun) {
//	const GRun* theRun = static_cast<const GRun*>(aRun);

	int thread_id = G4Threading::G4GetThreadId();
	int run       = aRun->GetRunID();

	if (IsMaster()) {
		//int neventsThisRun = theRun->GetNumberOfEventToBeProcessed();

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


		log->info(2, FUNCTION_NAME, " for Master for run id ", run, " in thread id: ", thread_id);






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
	}
	else {


		log->info(2, FUNCTION_NAME, " for Worker for run id ", run, " in thread id: ", thread_id);



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

	}
}


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
