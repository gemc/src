// gemc
#include "gRunAction.h"
#include "gRun.h"
#include "gutilities.h"
#include "gemcUtilities.h"
#include "gstreamerConventions.h"

// geant4
#include "globals.hh"
#include "G4Threading.hh"
#include "G4MTRunManager.hh"



// c++
using namespace std;

// Constructor for workers
GRunAction::GRunAction(GOptions *gopt, map<string, GDynamicDigitization *> *gDDGlobal, map<string, GStreamer *> *streamerFactoryMap) :
        G4UserRunAction(),                           // G4UserRunAction derived
        GStateMessage(gopt, "GRunAction", "grun"),  // GStateMessage derived
        goptions(gopt),
        gDigitizationGlobalMap(gDDGlobal),
        gstreamerFactoryMap(streamerFactoryMap) {
    verbosity = gopt->getVerbosityFor("grun");
    eventStreamVerbosity = gopt->getVerbosityFor("gstreamer_ev");
    frameStreamVerbosity = gopt->getVerbosityFor("gstreamer_fr");;

    logSummary("Instantiating GRunAction ");
    frameDuration = 64000;
    eventDuration = gutilities::getG4Number(gopt->getScalarString("eventTimeSize"));

    stream = gopt->getSwitch("stream");
}


// Destructor
GRunAction::~GRunAction() {
    // need to delete all data here
    // done with data, deleting it
    //for ( auto* frameDataCollection: frameRunData )  { delete frameDataCollection; }
}

// TODO: this is not local?
G4Run *GRunAction::GenerateRun() {
    logSummary("GRunAction GenerateRun ");

    return new GRun(goptions, gDigitizationGlobalMap);
}

// executed after BeamOn
void GRunAction::BeginOfRunAction(const G4Run *aRun) {
    if (IsMaster()) {
        string logMessage = "BeginOfRunAction Master for run id " + to_string(aRun->GetRunID()) + " in g4thread " + to_string(G4Threading::G4GetThreadId());
        logSummary(logMessage);
    } else {
        string logMessage = "BeginOfRunAction Local for run id " + to_string(aRun->GetRunID()) + " in g4thread " + to_string(G4Threading::G4GetThreadId());
        logSummary(logMessage);
    }
}

void GRunAction::EndOfRunAction(const G4Run *aRun) {
    const GRun *theRun = static_cast<const GRun *>(aRun);

    if (IsMaster()) {

        int neventsThisRun = theRun->GetNumberOfEventToBeProcessed();

        // TODO: 2 more is too much we need some calculation here
        int nFramesToCreate = neventsThisRun * eventDuration / frameDuration + 2;

        if (stream) {
            if (frameStreamVerbosity >= GVERBOSITY_SUMMARY) {
                cout << SROLOGHEADER << " current nframes in the buffer: " << frameRunData.size() << ", new frames to create: " << nFramesToCreate;
                cout << ", last frame id created: " << lastFrameCreated << endl;
            }

            for (int f = lastFrameCreated; f < lastFrameCreated + nFramesToCreate; f++) {
                GFrameDataCollectionHeader *gframeHeader = new GFrameDataCollectionHeader(f + 1, frameDuration, verbosity);
                GFrameDataCollection *frameData = new GFrameDataCollection(gframeHeader, verbosity);
                frameRunData.push_back(frameData);
            }

            lastFrameCreated += nFramesToCreate;
            if (frameStreamVerbosity >= GVERBOSITY_SUMMARY) {
                cout << SROLOGHEADER << nFramesToCreate << " new frames, buffer size is now " << frameRunData.size();
                cout << ", last frame id created: " << lastFrameCreated << endl;
            }
        }

        string logMessage = "EndOfRunAction Master, run " + to_string(aRun->GetRunID()) + " in g4thread " + to_string(G4Threading::G4GetThreadId());
        logMessage += ", data size:  " + to_string(theRun->getRunData().size());
        logSummary(logMessage);
        logSummary("Total number of events this run: " + to_string(theRun->GetNumberOfEvent()));

        // looping over output factories
        for (auto [factoryName, streamerFactory]: *gstreamerFactoryMap) {
            if (streamerFactory->getStreamType() == "event") {
                logSummary("Writing event data using streamer factory >" + factoryName + "<");
                map<string, bool> streamReport = streamerFactory->publishEventRunData(goptions, theRun->getRunData());

                for (auto [reportName, result]: streamReport) {
                    string resultString = result ? " success" : " failure";
                    logSummary("Factory <" + factoryName + "> " + reportName + resultString);
                }
            }
        }


        // looping over run data and filling frameRunData
        // need to remember last event number here
        if (stream) {
            for (auto eventDataCollection: theRun->getRunData()) {

                int absoluteEventNumber = eventIndex + eventDataCollection->getEventNumber();

                // filling frameRunData with this eventDataCollection
                for (auto [detectorName, gdataCollection]: *eventDataCollection->getDataCollectionMap()) {
                    for (auto hitDigitizedData: *gdataCollection->getDigitizedData()) {

                        int timeAtelectronic = hitDigitizedData->getTimeAtElectronics();
                        if (timeAtelectronic != TIMEATELECTRONICSNOTDEFINED) {
                            int frameIndex = eventFrameIndex(absoluteEventNumber, timeAtelectronic);
                            frameRunData[frameIndex]->addIntegralPayload(formPayload(hitDigitizedData), verbosity);
                        }
                    }
                }
            }
        }

        // done with data, deleting it
        for (auto *eventDataCollection: theRun->getRunData()) { delete eventDataCollection; }


        // now flushing all frames past eventIndex

        if (stream) {
            // updating eventIndex
            eventIndex += neventsThisRun;

            for (auto [factoryName, streamerFactory]: *gstreamerFactoryMap) {
                if (streamerFactory->getStreamType() == "stream" && frameRunData.size() > 0) {

                    // need to look for additional frame to flush
                    int nFramesToFlush = nFramesToCreate - 2;

                    if (frameStreamVerbosity >= GVERBOSITY_SUMMARY) {
                        cout << SROLOGHEADER << "number of frames to flush: " << nFramesToFlush << endl;
                    }
                    for (auto fid = 0; fid < nFramesToFlush; fid++) {
                        logSummary("Streaming frame id <" + to_string(frameRunData.front()->getFrameID()) + " using streamer factory >" + factoryName + "<");
                        streamerFactory->publishFrameRunData(goptions, frameRunData.front());
                        delete frameRunData.front();
                        frameRunData.erase(frameRunData.begin());
                    }
                }
            }
        }

    } else {
        // not in master thread
        string logMessage = "EndOfRunAction Local, run " + to_string(aRun->GetRunID()) + " in g4thread " + to_string(G4Threading::G4GetThreadId());
        logMessage += ", data size:  " + to_string(theRun->getRunData().size());
        logSummary(logMessage);
        logSummary("Total number of events this thread: " + to_string(theRun->GetNumberOfEvent()));
    }

}


// determine the frame ID based on event number, eventDuration, frameDuration and number of threads
// add frameData to frameRunData if it's not present
int GRunAction::eventFrameIndex(int eventNumber, float timeAtElectronics) {

    int absoluteHitTime = eventNumber * eventDuration + timeAtElectronics;
    int frameID = absoluteHitTime / frameDuration + 1;
    int frameIndex = -1;

    // cout << "eventNumber: " << eventNumber << ", absoluteHitTime: " << absoluteHitTime << ", frameID: " << frameID << endl;

    for (size_t f = 0; f < frameRunData.size(); f++) {
        if (frameRunData[f]->getFrameID() == frameID) {
            frameIndex = (int) f;
        }
    }
    // cout << "eventNumber: " << eventNumber << ", absoluteHitTime: " << absoluteHitTime << ", frameIndex: " << frameIndex << endl;

    return frameIndex;
}

vector<int> GRunAction::formPayload(GDigitizedData *digitizedData) {
    vector<int> payload;

    int crate = digitizedData->getIntObservable(CRATESTRINGID);
    int slot = digitizedData->getIntObservable(SLOTSTRINGID);
    int channel = digitizedData->getIntObservable(CHANNELSTRINGID);
    int q = digitizedData->getIntObservable(CHARGEATELECTRONICS);
    int time = digitizedData->getIntObservable(TIMEATELECTRONICS);

    payload.push_back(crate);
    payload.push_back(slot);
    payload.push_back(channel);
    payload.push_back(q);
    payload.push_back(time);

    return payload;
}

bool GRunAction::findFrameID(int fid) {
    for (auto frame: frameRunData) {
        if (frame->getFrameID() == fid) {
            return true;
        }
    }
    return false;
}
