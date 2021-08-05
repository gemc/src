// gemc
#include "gPrimaryGeneratorAction.h"
#include "gActionInitialization.h"
#include "gRunAction.h"
#include "gEventAction.h"

// glibrary
#include "gfactory.h"
#include "gstreamerOptions.h"
using namespace gstreamer;
//#include "gstring.h"
//using namespace gstring;

// c++
#include <iostream>
using namespace std;

GActionInitialization::GActionInitialization(GOptions* gopts, map<string, GDynamicDigitization*> *gDDGlobal) :
GStateMessage(gopts, "GActionInitialization", "verbosity"),  // GStateMessage derived
goptions(gopts),
gDigitizationGlobal(gDDGlobal)
{
	logSummary("Instantiating GActionInitialization ");
	int verbosity = goptions->getInt("verbosity");

	// gstreamerFactory
	gstreamerFactory = new map<string, GStreamer*>;

	// projecting options onto vector of JOutput
	vector<JOutput> joutputs = getJOutputs(gopts);


	// if any output is specified, loading its factory
	if(joutputs.size() > 0) {

		GManager gStreamerManager("GOutput", verbosity);

		// the available plugins names are formatted as "xxxGMedia".
		for(auto &joutput: joutputs) {
			string factory = joutput.format;
			string pluginName = gstreamerPluginNameFromFactory(factory);

			if(gstreamerFactory->find(factory) == gstreamerFactory->end()) {
				(*gstreamerFactory)[factory] = gStreamerManager.LoadAndRegisterObjectFromLibrary<GStreamer>(pluginName);
				(*gstreamerFactory)[factory]->setOutputName(joutput.name);
				(*gstreamerFactory)[factory]->openConnection();
			}
		}
	}
}

GActionInitialization::~GActionInitialization()
{
	// close output connections
	for(auto gsf: (*gstreamerFactory)) {
		// protecting against DL failure
		if(gsf.second != nullptr) {
			gsf.second->closeConnection();
		}
	}
}

void GActionInitialization::Build() const
{
	flowMessage("Thread Build");
	
	SetUserAction(new GRunAction(gopt, gDigitizationGlobal, gmediaFactory));
	SetUserAction(new GPrimaryGeneratorAction);
	SetUserAction(new GEventAction(gopt));
}

void GActionInitialization::BuildForMaster() const
{
	flowMessage("Master Build");
	SetUserAction(new GRunAction(gopt, gDigitizationGlobal, gmediaFactory));
}


