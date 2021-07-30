// gemc
#include "gPrimaryGeneratorAction.h"
#include "gActionInitialization.h"
#include "gRunAction.h"
#include "gEventAction.h"

// mlibrary
#include "gfactory.h"
#include "gstring.h"
using namespace gstring;

// c++
#include <iostream>
using namespace std;

GActionInitialization::GActionInitialization(GOptions* opt, map<string, GDynamic*> *gDigitization) :
GFlowMessage(opt, "GActionInitialization"),
gopt(opt),
gDigitizationGlobal(gDigitization)
{
	flowMessage("GActionInitialization Constructor");
	gmediaFactory = new map<string, GMedia*>;
	
	int verbosity = gopt->getInt("gemcv");
	
	string gmediaOption        = gopt->getString("output");
	auto cpos = gmediaOption.find(":");
	
	if(cpos == string::npos) {
		cout << GWARNING << " output option cannot be parsed. " << endl;
	}
	
	string fileNameWOExtension = gmediaOption.substr(0, cpos); // until :
	string requestedExtensions = gmediaOption.substr(cpos+1);  // after :

	vector<string> requestedMedias = getStringVectorFromStringWithDelimiter(requestedExtensions, ",");
	
	if(requestedMedias.size() > 0) {

		GManager gOutputManager(verbosity);

		// the available plugins names are formatted as "xxxGMedia".
		for(unsigned f=0; f<requestedMedias.size(); f++) {
			string pluginName = requestedMedias[f] + "GMedia";
			// need path here
			gOutputManager.registerDL(pluginName);

			(*gmediaFactory)[requestedMedias[f]] = gOutputManager.LoadObjectFromLibrary<GMedia>(pluginName);

			// set file name, open the connection
			// protect against DL loading failure
			if((*gmediaFactory)[requestedMedias[f]]  != nullptr) {
				(*gmediaFactory)[requestedMedias[f]]->setOutputName(fileNameWOExtension);
			}
		}
	}
}

GActionInitialization::~GActionInitialization()
{
	// close output connections
	for(auto gmf: (*gmediaFactory)) {
		// protecting against DL failure
		if(gmf.second != nullptr) {
			gmf.second->closeConnection();
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


