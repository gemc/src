#pragma once

// qt
#include <QCoreApplication>

// geant4
#include "G4MTRunManager.hh"
#include "G4UImanager.hh"

// cpp
#include <memory>

// gemc
#include "glogger.h"
#include "gsplash.h"
#include "gbatch_session.h"


namespace gemc {


inline std::unique_ptr<QCoreApplication>
makeQtApplication(int& argc, char* argv[], bool gui) {

	if (!gui) {
		auto gbatch = std::make_unique<GBatch_Session>();
		G4UImanager::GetUIpointer()->SetCoutDestination(gbatch.release()); // release ownership to Geant4

		return std::make_unique<QCoreApplication>(argc, argv);
	}
	return std::make_unique<QApplication>(argc, argv);
}

// return the number of cores from options.
// if 0 is given, returns max number of available cores
int get_nthreads(const std::shared_ptr<GOptions>& gopts, const std::shared_ptr<GLogger>& log);

std::vector<std::string> verbosity_commands(const std::shared_ptr<GOptions>& gopts, const std::shared_ptr<GLogger>& log);
std::vector<std::string> initial_commands(const std::shared_ptr<GOptions>& gopts, const std::shared_ptr<GLogger>& log);

// initialize G4MTRunManager
void run_manager_commands(const std::shared_ptr<GOptions>& gopts, const std::shared_ptr<GLogger>& log, const std::vector<std::string>& commands);

// apply initial UIM commands coming from, in order:
// - startingUIMCommands
// - PRAGMA TODO macro file
//void applyInitialUIManagerCommands(bool gui, int checkForOverlaps, int verbosity);

void start_random_engine(const std::shared_ptr<GOptions>& gopts, const std::shared_ptr<GLogger>& log);

}
