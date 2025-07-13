// gstreamer
#include "gstreamerROOTFactory.h"
#include "gstreamerConventions.h"

// root
#include "TROOT.h"
#include "TSystem.h"

// c__
#include <mutex> // for once_flag

// Static member definition
std::once_flag GstreamerRootFactory::rootInitFlag;

bool GstreamerRootFactory::openConnectionImpl() {

	// Trigger interpreter and global dictionary setup
	// this needs to be called in the main thread before any ROOT objects are created
	std::call_once(rootInitFlag, []() {
		TClass::GetClass("TObject"); // triggers interpreter
		gSystem->Load("libCore");    // optional
		// log->info(0, "GstreamerRootFactory: ROOT interpreter initialized");
	});

	rootfile = std::make_unique<TFile>(filename().c_str(), "RECREATE");
	rootfile->cd();
	TDirectory::TContext ctx(rootfile.get()); // thread-local context

	gRootTrees = new std::map<std::string, GRootTree*>;

	if (!rootfile->IsOpen()) {
		log->error(ERR_CANTOPENOUTPUT, "GstreamerRootFactory: could not open file " + filename());
	}
	else {
		log->info(0, "GstreamerRootFactory: opened file " + filename());
	}

	return true;
}

bool GstreamerRootFactory::closeConnectionImpl() {
	TDirectory::TContext ctx(rootfile.get()); // thread-local context

	if (rootfile->IsOpen()) {
		rootfile->Write();
		rootfile->Close();
	}

	if (rootfile->IsOpen()) { log->error(ERR_CANTOPENOUTPUT, "GstreamerRootFactory: could not close file " + filename()); }

	delete gRootTrees;

	return true;
}
