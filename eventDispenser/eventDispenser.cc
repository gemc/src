// Implements EventDispenser: run-weight parsing, event distribution, and per-run dispatch through Geant4.
//
// Doxygen documentation for the public API is maintained in eventDispenser.h.
// This implementation file keeps only short, non-Doxygen summaries and inline clarifying comments.

#include "eventDispenserConventions.h"
#include "eventDispenser_options.h"
#include "eventDispenser.h"
#include "gdynamicdigitizationConventions.h"

// c++
#include <fstream>
#include <random>
#include <utility>

// geant4
#include "G4UImanager.hh"

using namespace std;

// Constructor summary:
// - Reads configuration (number of events, run number, optional run-weight file).
// - Builds runWeights/runEvents/listOfRuns when weights are provided.
// - Otherwise, falls back to single-run mode.
EventDispenser::EventDispenser(const std::shared_ptr<GOptions>&                                 gopt,
                               const std::shared_ptr<const gdynamicdigitization::dRoutinesMap>& gdynamicDigitizationMap)
	: GBase(gopt, EVENTDISPENSER_LOGGER), gDigitizationMap(gdynamicDigitizationMap) {
	// Retrieve configuration parameters from GOptions.
	string filename  = gopt->getScalarString("run_weights");
	userRunno        = gopt->getScalarInt("run");
	neventsToProcess = gopt->getScalarInt("n");

	// If there are no events to process, keep the object in an initialized-but-idle state.
	if (neventsToProcess == 0) return;

	// If no file is provided, use the user-specified run number (single-run mode).
	if (filename == UNINITIALIZEDSTRINGQUANTITY && neventsToProcess > 0) {
		runEvents[userRunno] = neventsToProcess;
		return;
	}
	else {
		// Multi-run mode: a filename was specified; attempt to open the run weights input file.
		ifstream in(filename.c_str());
		if (!in) {
			// Keep behavior unchanged: log error and continue with an empty distribution.
			log->error(ERR_EVENTDISTRIBUTIONFILENOTFOUND,
			           "Error: can't open run weights input file >", filename, "<. Check your spelling. Exiting.");
		}
		else {
			log->info(1, "Loading run weights from ", filename);

			// Read "run weight" pairs, one per line.
			// The order of insertion into listOfRuns reflects the file order and may be used by clients.
			int    run;
			double weight;
			while (in >> run >> weight) {
				listOfRuns.push_back(run);
				runWeights[run] = weight;
				runEvents[run]  = 0; // initialize per-run counters before distribution
			}

			// Distribute the total number of events among runs according to their weights.
			distributeEvents(neventsToProcess);
		}
		in.close();

		// Log summary information: overall distribution table.
		log->info(0, "EventDispenser initialized with ", neventsToProcess, " events distributed among ",
		          runWeights.size(), " runs:");
		log->info(0, " run\t weight\t  n. events");
		for (const auto& weight : runWeights) {
			log->info(0, " ", weight.first, "\t ", weight.second, "\t  ", runEvents[weight.first]);
		}
	}
}


// setNumberOfEvents summary:
// - Clears any existing distribution and assigns all events to the user-selected run number.
void EventDispenser::setNumberOfEvents(int nevents_to_process) {
	runEvents.clear();
	runEvents[userRunno] = nevents_to_process;
}


// distributeEvents summary:
// - Performs stochastic sampling to convert runWeights into integer runEvents counts.
void EventDispenser::distributeEvents(int nevents_to_process) {
	// Set up a random number generator drawing from U[0, 1].
	random_device               randomDevice;
	mt19937                     generator(randomDevice());
	uniform_real_distribution<> randomDistribution(0, 1);

	// For each event, select a run by comparing a random draw to the cumulative weight intervals.
	// This assumes runWeights values represent fractions or relative weights normalized to sum to 1.
	for (int i = 0; i < nevents_to_process; i++) {
		double randomNumber = randomDistribution(generator);

		double cumulativeWeight = 0;
		for (const auto& weight : runWeights) {
			cumulativeWeight += weight.second;
			if (randomNumber <= cumulativeWeight) {
				runEvents[weight.first]++;
				break;
			}
		}
	}
}


// getTotalNumberOfEvents summary:
// - Sums all per-run event counts from runEvents.
int EventDispenser::getTotalNumberOfEvents() const {
	int totalEvents = 0;
	for (auto rEvents : runEvents) { totalEvents += rEvents.second; }
	return totalEvents;
}


// processEvents summary:
// - Iterates the run allocation.
// - For each run, loads run-dependent constants/TT via digitization routines (if run changed).
// - Dispatches the events to Geant4 via \c /run/beamOn.
int EventDispenser::processEvents() {
	// Get the Geant4 UI manager pointer used to apply macro commands.
	G4UImanager* g4uim = G4UImanager::GetUIpointer();

	// Iterate over each run in the run events map.
	for (auto& run : runEvents) {
		int runNumber = run.first;
		int nevents   = run.second;

		// Load constants and translation tables if the run number has changed.
		if (runNumber != currentRunno) {
			// Iterate the (plugin name -> digitization routine) map.
			// digiRoutine is a std::shared_ptr<GDynamicDigitization>.
			for (const auto& [plugin, digiRoutine] : *gDigitizationMap) {
				log->debug(NORMAL, FUNCTION_NAME, "Calling ", plugin, " loadConstants for run ", runNumber);
				if (digiRoutine->loadConstants(runNumber, variation) == false) {
					log->error(ERR_LOADCONSTANTFAIL,
					           "Failed to load constants for ", plugin, " for run ", runNumber, " with variation ",
					           variation);
				}

				log->debug(NORMAL, FUNCTION_NAME, "Calling ", plugin, " loadTT for run ", runNumber);
				if (digiRoutine->loadTT(runNumber, variation) == false) {
					log->error(ERR_LOADTTFAIL,
					           "Failed to load translation table for ", plugin, " for run ", runNumber,
					           " with variation ", variation);
				}
			}
			currentRunno = runNumber;
		}

		log->info(1, "Starting run ", runNumber, " with ", nevents, " events.");

		// Dispatch all events for this run in a single call.
		// The command string is a standard Geant4 UI command: \c /run/beamOn <N>.
		log->info(1, "Processing ", nevents, " events in one go");
		g4uim->ApplyCommand("/run/beamOn " + to_string(nevents));

		log->info(1, "Run ", runNumber, " done with ", nevents, " events");
	}

	return 1;
}
