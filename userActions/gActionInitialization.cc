// gemc
#include "gPrimaryGeneratorAction.h"
#include "gActionInitialization.h"
#include "gRunAction.h"
#include "gEventAction.h"

// glibrary
#include "gfactory.h"
#include "gstreamerOptions.h"
using namespace gstreamer;


// c++
#include <iostream>
using namespace std;

GActionInitialization::GActionInitialization(GOptions* gopts, map<string, GDynamicDigitization*> *gDDGlobal) :
GStateMessage(gopts, "GActionInitialization", "verbosity"),  // GStateMessage derived
goptions(gopts),
gDigitizationGlobalMap(gDDGlobal)
{
	logSummary("Instantiating GActionInitialization ");
	int verbosity = goptions->getInt("verbosity");

	// gstreamerFactory
	gstreamerFactoryMap = new map<string, GStreamer*>;

	// projecting options onto vector of JOutput
	vector<JOutput> joutputs = getJOutputs(gopts);

	// if any output is specified, loading its factory
	if(joutputs.size() > 0) {

		GManager gStreamerManager("GOutput", verbosity);

		// the available plugins names are formatted as "xxxGMedia".
		for(auto &joutput: joutputs) {
			string factory        = joutput.format;
			string outputFileName = joutput.name;
			string streamType     = joutput.type;

			if( factory != UNINITIALIZEDSTRINGQUANTITY && outputFileName != UNINITIALIZEDSTRINGQUANTITY) {
				string pluginName = gstreamerPluginNameFromFactory(factory);

				if(gstreamerFactoryMap->find(factory) == gstreamerFactoryMap->end()) {
					(*gstreamerFactoryMap)[factory] = gStreamerManager.LoadAndRegisterObjectFromLibrary<GStreamer>(pluginName);
					(*gstreamerFactoryMap)[factory]->setOutputName(outputFileName);
					(*gstreamerFactoryMap)[factory]->setStreamType(streamType);
					(*gstreamerFactoryMap)[factory]->openConnection();
				}
			}
		}

		// done with gStreamerManager
	//	gStreamerManager.clearDLMap();

	}
}

// Destructor:
// delete gstreamerFactoryMap
GActionInitialization::~GActionInitialization()
{
	// close output connections
	for(auto gsf: (*gstreamerFactoryMap)) {
		// protecting against DL failure
		if(gsf.second != nullptr) {
			gsf.second->closeConnection();
		}
	}
}

void GActionInitialization::Build() const
{
	logSummary("GActionInitialization Thread Build ");

	SetUserAction(new GPrimaryGeneratorAction());
	SetUserAction(new GRunAction(goptions, gDigitizationGlobalMap, gstreamerFactoryMap));
	SetUserAction(new GEventAction(goptions));
}

void GActionInitialization::BuildForMaster() const
{
	logSummary("GActionInitialization Master Build ");

	SetUserAction(new GRunAction(goptions, gDigitizationGlobalMap, gstreamerFactoryMap));
}


