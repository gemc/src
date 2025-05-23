// gstreamer
#include "gstreamerROOTFactory.h"
#include "gstreamerConventions.h"

bool GstreamerRootFactory::openConnection(const std::shared_ptr<GLogger>& log)
{
	rootfile = new TFile(std::string(gstreamer_definitions.name).c_str(), "RECREATE");
	gRootTrees = new std::map<std::string, GRootTree*>;

	if (!rootfile->IsOpen()) {
		log->error(ERR_CANTOPENOUTPUT, "GstreamerRootFactory: could not open file " + gstreamer_definitions.name);
	}

	return true;
}

bool GstreamerRootFactory::closeConnection(const std::shared_ptr<GLogger>& log)
{
	rootfile->Write();
	rootfile->Close();

	if (!rootfile->IsOpen()) {
		log->error(ERR_CANTOPENOUTPUT, "GstreamerRootFactory: could not clos file " + gstreamer_definitions.name);
	}

	delete rootfile;
	delete gRootTrees;
	
	return true;
}
