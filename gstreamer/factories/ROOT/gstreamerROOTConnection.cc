// gstreamer
#include "gstreamerROOTFactory.h"
#include "gstreamerConventions.h"

// root
#include <TFile.h>

// thread local
// Non-Doxygen implementation file: behavior is documented in the header.
bool GstreamerRootFactory::openConnection() {
	log->debug(NORMAL, "GstreamerRootFactory::openConnection -> opening file " + filename());

	rootfile = std::make_unique<TFile>(filename().c_str(), "RECREATE");

	if (rootfile->IsZombie()) {
		log->error(
			ERR_CANTOPENOUTPUT, "GstreamerRootFactory: could not create file " + filename() + " (file is a zombie)");
	}
	else { log->info(0, "GstreamerRootFactory: opened file " + filename()); }

	return true;
}

bool GstreamerRootFactory::closeConnectionImpl() {
	// Persist file content and detach trees before closing.
	rootfile->Write();
	gRootTrees.clear(); // clear all trees to detach from the file. W/o this we have crash on exit
	rootfile->Close();

	return true;
}
