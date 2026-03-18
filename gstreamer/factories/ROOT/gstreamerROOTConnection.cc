// gstreamer
#include "gstreamerROOTFactory.h"
#include "gstreamerConventions.h"

// root
#include <TFile.h>

// Implementation summary:
// Manage the lifetime of the ROOT file that owns all trees created by the plugin.

bool GstreamerRootFactory::openConnection() {
	log->debug(NORMAL, "GstreamerRootFactory::openConnection -> opening file " + filename());

	rootfile = std::make_unique<TFile>(filename().c_str(), "RECREATE");

	if (rootfile->IsZombie()) {
		log->error(
			ERR_CANTOPENOUTPUT, "GstreamerRootFactory: could not create file " + filename() + " (file is a zombie)");
	}

	log->info(1, SFUNCTION_NAME, "GstreamerRootFactory: opened file " + filename());

	return true;
}

bool GstreamerRootFactory::closeConnectionImpl() {
	// The public closeConnection() wrapper already flushes buffered events before this method runs.

	// Persist all tree content, then destroy tree wrappers before closing the file so ROOT
	// object ownership is torn down in a controlled order.
	rootfile->Write();
	gRootTrees.clear();
	rootfile->Close();

	log->info(1, SFUNCTION_NAME, "GstreamerRootFactory: closed file " + filename());

	return true;
}