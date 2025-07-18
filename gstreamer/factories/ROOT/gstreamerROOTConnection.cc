// gstreamer
#include "gstreamerROOTFactory.h"
#include "gstreamerConventions.h"

// root
#include <TFile.h>

bool GstreamerRootFactory::openConnection() {

	log->debug(NORMAL, "GstreamerRootFactory::openConnection -> opening file " + filename());

	rootfile = std::make_unique<TFile>(filename().c_str(), "RECREATE");

	if (rootfile->IsZombie()) { log->error(ERR_CANTOPENOUTPUT, "GstreamerRootFactory: could not create file " + filename() + " (file is a zombie)"); }
	else { log->info(0, "GstreamerRootFactory: opened file " + filename()); }

	return true;
}

bool GstreamerRootFactory::closeConnection() {

	// in case there are still events in the buffer
	flushEventBuffer(); // base version

	if (rootfile && rootfile->IsOpen()) {
		rootfile->cd();
		// write all trees to file
		for (auto& [name, groottree] : gRootTrees) { if (groottree != nullptr) { groottree->writeToFile(); } }

	}
	// 	rootfile->Write();
	// 	rootfile->Close();
	// }
	//
	// if (rootfile->IsZombie()) { log->error(ERR_CANTOPENOUTPUT, "GstreamerRootFactory: file is a zombie"); }

	return true;
}
