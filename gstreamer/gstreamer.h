#ifndef  GSTREAMER_H
#define  GSTREAMER_H  1

// glibrary
#include "event/gEventDataCollection.h"
#include "frame/gFrameDataCollection.h"
#include "gfactory.h"
#include "goptions.h"

// c++
#include <string>

class GStreamer {

public:
    virtual ~GStreamer() = default;

    virtual bool openConnection() { return false; }  // in GActionInitialization constructor
    virtual bool closeConnection() { return false; }  // in GActionInitialization destructor

    // called in GRunAction::EndOfRunAction
    // runs the protected virtual methods below to write events from a run to file
    // the key is the routine name + sensitive detector name
    map<string, bool> publishEventRunData(const GOptions *gopts, const vector<GEventDataCollection *> runData);

    // called in GRunAction::EndOfRunAction
    // runs the protected virtual methods below to write frames from a run to file
    // the key is the routine name + frame streamer id
    map<string, bool> publishFrameRunData(const GOptions *gopts, const GFrameDataCollection *frameRunData);

    void setOutputName(const string output) { outputFileName = output; }

    void setStreamType(const string stype) { streamType = stype; }

    inline string const getStreamType() const { return streamType; }

protected:

    string outputFileName = UNINITIALIZEDSTRINGQUANTITY;
    string streamType = UNINITIALIZEDSTRINGQUANTITY;

    // event virtual methods called by publishRunData, in order
    // --------------------------------------------------------

    virtual bool startEvent([[maybe_unused]] const GEventDataCollection *eventData) { return false; }

    virtual bool publishEventHeader([[maybe_unused]] const GEventDataCollectionHeader *gheader) { return false; }

    // vector index is hit number
    virtual bool publishEventTrueInfoData ([[maybe_unused]] const string detectorName, [[maybe_unused]] const vector<GTrueInfoData *> *trueInfoData) { return false; }

    virtual bool publishEventDigitizedData([[maybe_unused]] const string detectorName, [[maybe_unused]] const vector<GDigitizedData *> *digitizedData) { return false; }

    virtual bool endEvent([[maybe_unused]] const GEventDataCollection *eventData) { return false; }

    // stream virtual methods
    virtual bool startStream([[maybe_unused]] const GFrameDataCollection *frameRunData) { return false; }

    virtual bool publishFrameHeader([[maybe_unused]] const GFrameDataCollectionHeader *gframeHeader) { return false; }

    virtual bool publishPayload([[maybe_unused]] const vector<GIntegralPayload *> *payload) { return false; }

    virtual bool endStream([[maybe_unused]] const GFrameDataCollection *frameRunData) { return false; }


public:

    // method to dynamically load factories
    static GStreamer *instantiate(const dlhandle handle) {

        if (handle == nullptr) return nullptr;

        void *maker = dlsym(handle, "GStreamerFactory");

        if (maker == nullptr) return nullptr;

        typedef GStreamer *(*fptr)();

        // static_cast not allowed here
        // see http://stackoverflow.com/questions/573294/when-to-use-reinterpret-cast
        // need to run the DLL GMediaFactory function that returns the factory
        fptr func = reinterpret_cast<fptr>(reinterpret_cast<void *>(maker));

        return func();
    }


};


#endif
