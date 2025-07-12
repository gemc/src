// gstreamer
#include "gstreamerROOTFactory.h"
#include "gstreamerConventions.h"

bool GstreamerRootFactory::openConnectionImpl() {

	rootfile = std::make_unique<TFile>(filename().c_str(), "RECREATE");
	rootfile->cd();
	TDirectory::TContext ctx(rootfile.get());  // thread-local context

	gRootTrees = new std::map<std::string, GRootTree*>;

	if (!rootfile->IsOpen()) { log->error(ERR_CANTOPENOUTPUT, "GstreamerRootFactory: could not open file " + filename()); }
	else { log->info(0, "GstreamerRootFactory: opened file " + filename()); }

	return true;
}

bool GstreamerRootFactory::closeConnectionImpl() {
	TDirectory::TContext ctx(rootfile.get());  // thread-local context

	if (rootfile->IsOpen()) {
		rootfile->Write();
		rootfile->Close();
	}

	if ( rootfile->IsOpen() ) { log->error(ERR_CANTOPENOUTPUT, "GstreamerRootFactory: could not close file " + filename()); }

	delete gRootTrees;

	return true;
}
