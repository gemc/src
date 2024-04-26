// gstreamer
#include "gstreamerROOTFactory.h"

bool GstreamerRootFactory::openConnection()
{
	rootfile = new TFile(string(outputFileName).c_str(), "RECREATE");
	gRootTrees = new map<string, GRootTree*>;


	// PRAGMA: need to check if file opened successfully
	return true;
}

bool GstreamerRootFactory::closeConnection()
{
	rootfile->Write();
	rootfile->Close();

	delete rootfile;
	delete gRootTrees;
	
	// PRAGMA: need to check if file closed successfully
	return true;
}



