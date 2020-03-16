// mlibrary
#include "goptions.h"

// gemc
#include "gLog.h"

// c++
#include <iostream>


GSession::GSession()
{
	logFile.open("MasterGeant4.log");
	errFile.open("MasterGeant4.err");
}

G4int GSession::ReceiveG4cout(const G4String& coutString)
{
	logFile << coutString << flush;
	return 0;
}

G4int GSession::ReceiveG4cerr(const G4String& cerrString)
{
	errFile << cerrString << flush;
	return 0;
}


map<string, GOption> GFlowMessage::defineOptions()
{
	map<string, GOption> optionsMap;

	optionsMap["gflowv"] = GOption("GFlowMessage Verbosity", 0, "verbosity");
	optionsMap["gflowv"].addHelp("Possible values:\n");
	optionsMap["gflowv"].addHelp(GVERBOSITY_SILENT_D);
	optionsMap["gflowv"].addHelp(GVERBOSITY_SUMMARY_D);
	optionsMap["gflowv"].addHelp(GVERBOSITY_DETAILS_D);
	optionsMap["gflowv"].addHelp(GVERBOSITY_ALL_D);
	
	return optionsMap;
}
