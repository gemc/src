/**
 * \file eventDispenser.cc
 * \brief Implements the EventDispenser class which distributes events among runs.
 *
 * \mainpage Event Dispenser Module
 *
 * \section intro_sec Introduction
 * The Event Dispenser module is responsible for distributing simulation events
 * among different runs based on a user–defined weighting scheme. It reads run weight
 * information from a file (if provided) or uses options provided via GOptions. It then
 * randomly assigns events to runs and executes them using Geant4 commands.
 *
 * \section details_sec Details
 * The module initializes the run weight maps, distributes events randomly based on the weights,
 * and processes events by calling Geant4 UI commands (e.g., /run/beamOn). The class also logs
 * progress and summary information.
 *
 * \section usage_sec Usage
 * To use the EventDispenser, instantiate it with a pointer to GOptions and a global map of
 * GDynamicDigitization plugins. Call processEvents() to run the events.
 *
 * \author Your Name
 * \date YYYY-MM-DD
 */

#include "eventDispenserConventions.h"
#include "eventDispenser_options.h"
#include "eventDispenser.h"

// c++
#include <fstream>
#include <random>

// geant4
#include "G4UImanager.hh"

using namespace std;

//
// Constructor: Initializes the EventDispenser using options from GOptions and a pointer to the global
// GDynamicDigitization plugins map.
//
EventDispenser::EventDispenser(GOptions *gopt, map<string, GDynamicDigitization *> *gDDGlobal)
		: log(std::make_shared<GLogger>(gopt, EVENTDISPENSER_LOGGER)) , gDigitizationGlobal(gDDGlobal) {

	log->debug(CONSTRUCTOR, "EventDispenser");

	// Retrieve configuration parameters from GOptions.
	string filename = gopt->getScalarString("run_weights");
	userRunno = gopt->getScalarInt("run");
	nEventBuffer = gopt->getScalarInt("n_event_buffer");
	neventsToProcess = gopt->getScalarInt("n");

	// If there are no events to process, nothing more to do.
	if (neventsToProcess == 0) return;

	// If no file is provided, use the user-specified run number.
	if (filename == UNINITIALIZEDSTRINGQUANTITY && neventsToProcess > 0) {
		// Only one run is defined via the option.
		runEvents[userRunno] = neventsToProcess;
		return;
	} else {
		// A filename was specified; attempt to open the run weights input file.
		ifstream in(filename.c_str());
		if (!in) {
			log->error(EC__EVENTDISTRIBUTIONFILENOTFOUND, "Error: can't open run weights input file ", filename, ". Check your spelling. Exiting.");
		} else {
			log->info(1, "Loading run weights from ", filename);
			// Fill the run weight map by reading each line from the file.
			int run;
			double weight;
			// Use a robust loop to read pairs from the file.
			while (in >> run >> weight) {
				listOfRuns.push_back(run);
				runWeights[run] = weight;
				runEvents[run] = 0;
			}
			// Distribute the total number of events among the runs according to the weights.
			distributeEvents(neventsToProcess);
		}
		in.close();

		// Log summary information.
		log->info(0, "EventDispenser initialized with ", neventsToProcess, " events distributed among ", runWeights.size(), " runs:");
		log->info(0, " run\t weight\t  n. events");
		for (const auto &weight: runWeights) {
			log->info(0, " ", weight.first, "\t ", weight.second, "\t  ", runEvents[weight.first]);
		}
	}
}

//
// Sets the total number of events to process. Clears previous run events and assigns all events to the user run.
//
void EventDispenser::setNumberOfEvents(int nevents_to_process) {
	runEvents.clear();
	runEvents[userRunno] = nevents_to_process;
}

//
// Randomly distributes events among runs according to the weights read from the input file.
//
void EventDispenser::distributeEvents(int nevents_to_process) {
	// Initialize a progress bar for visual feedback.
	//TextProgressBar bar(50, string(EVENTDISPENSERLOGMSGITEM) + " Distributing events according to run weights ", 0, nevents_to_process);

	// Set up a random number generator.
	random_device randomDevice;
	mt19937 generator(randomDevice());
	uniform_real_distribution<> randomDistribution(0, 1);

	// Loop over each event and assign it to a run based on the cumulative weight.
	for (int i = 0; i < nevents_to_process; i++) {
		double randomNumber = randomDistribution(generator);

		double cumulativeWeight = 0;
		for (const auto &weight: runWeights) {
			cumulativeWeight += weight.second;
			if (randomNumber <= cumulativeWeight) {
				runEvents[weight.first]++;
				break;
			}
		}
	//	bar.setProgress(i);
	}
}

//
// Sums the number of events assigned to all runs.
//
int EventDispenser::getTotalNumberOfEvents() {
	int totalEvents = 0;
	for (auto rEvents: runEvents) {
		totalEvents += rEvents.second;
	}
	return totalEvents;
}

//
// Processes events by iterating over runs, initializing plugins, and executing Geant4 commands.
//
int EventDispenser::processEvents() {
	// Get the Geant4 UI manager pointer.
	G4UImanager *g4uim = G4UImanager::GetUIpointer();
	// Set the progress print command based on the elog level.
	g4uim->ApplyCommand("/run/printProgress " + to_string(elog));

	// Iterate over each run in the run events map.
	for (auto &run: runEvents) {
		int runNumber = run.first;
		int nevents = run.second;

		// Load constants and translation table if the run number has changed.
		if (runNumber != currentRunno) {
			for (auto [digitizationName, digiRoutine] : (*gDigitizationGlobal)) {
				log->debug(NORMAL, "Calling ", digitizationName, " loadConstants for run ", runNumber);
				digiRoutine->loadConstants(runNumber, variation);

				log->debug(NORMAL, "Calling ", digitizationName, " loadTT for run ", runNumber);
				digiRoutine->loadTT(runNumber, variation);
			}
			currentRunno = runNumber;
		}

		log->info(1, "Starting run ", runNumber, " with ", nevents, " events. Event buffer is ", nEventBuffer);

		// If events are fewer than the buffer size, process all in one go.
		if (nevents <= nEventBuffer) {
			log->info(1, "Processing ", nevents, " events in one go");
			g4uim->ApplyCommand("/run/initialize");
			g4uim->ApplyCommand("/run/beamOn " + to_string(nevents));
		} else {
			// Otherwise, process events in sub-runs.
			int nsubRuns = nevents / nEventBuffer;
			int totalSoFar = 0;
			int lastSubRunNEvents = nevents % nEventBuffer;
			int ntotalSubRuns = (lastSubRunNEvents > 0 ? nsubRuns + 1 : nsubRuns);
			for (int s = 0; s < nsubRuns; s++) {
				log->info(1, "Processing sub run ", s + 1, " out of ", ntotalSubRuns, " with ", nEventBuffer, " events. Total events so far: ", totalSoFar);
				g4uim->ApplyCommand("/run/initialize");
				g4uim->ApplyCommand("/run/beamOn " + to_string(nEventBuffer));
				totalSoFar += nEventBuffer;
			}
			if (lastSubRunNEvents > 0) {
				log->info(1, "Processing sub run ", nsubRuns + 1, " with ", lastSubRunNEvents, " events");
				g4uim->ApplyCommand("/run/beamOn " + to_string(lastSubRunNEvents));
			}
		}

		log->info(1, "Run ", runNumber, " done with ", nevents, " events");
	}

	return 1;
}
