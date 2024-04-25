// eventDispenser 
#include "eventDispenserConventions.h"
#include "eventDispenser.h"

// c++
#include <fstream>
#include <random>
using namespace std;

// glibrary
#include "textProgressBar.h"

// geant4
#include "G4UImanager.hh"


EventDispenser::EventDispenser(GOptions* gopt, map<string, GDynamicDigitization*> *gDDGlobal) : gDigitizationGlobal(gDDGlobal) {
	verbosity        = gopt->getInt("edistv");
	string filename  = gopt->getString("runWeightsFile");
	//variation        = gopt->getString("dVariation");
	userRunno        = gopt->getInt("userRunno");
	nEventBuffer     = gopt->getInt("maxebuffer");
	neventsToProcess = gopt->getInt("n");
	//elog             = gopt->getInt("elog");

	// nothing to do here
	if(neventsToProcess == 0) return;

	// no filename
	// run number from options
	// number of events from options
	if(filename == UNINITIALIZEDSTRINGQUANTITY && neventsToProcess > 0 ) {
		// only one run, defined in the option
		runEvents[userRunno] = neventsToProcess;
		return;
	} else {
		// filename specified, reading it
		ifstream in(filename.c_str());
		if(!in) {
			cerr << EVENTDISPENSERLOGMSGITEM << " Error: can't open run weights input file " << filename << ". Check your spelling. Exiting. " << endl;
			gexit(EC__EVENTDISTRIBUTIONFILENOTFOUND);
		} else {
			if(verbosity >= GVERBOSITY_SUMMARY) {
				cout << EVENTDISPENSERLOGMSGITEM << " Loading run weights from " << filename << endl;
			}
			// filling weight map
			while (!in.eof()) {
				int run;
				double weight;
				in >> run >> weight;
				listOfRuns.push_back(run);
				runWeights[run] = weight;
				runEvents[run] = 0;
			}
			// distribute events according to weights
			distributeEvents(neventsToProcess);
		}
		in.close();

		if(verbosity >= GVERBOSITY_SUMMARY) {
			printRunsDetails(neventsToProcess);
		}
	}
}

void EventDispenser::setNumberOfEvents(int nevents_to_process) {
	runEvents.clear();
	runEvents[userRunno] = nevents_to_process;
}


void EventDispenser::distributeEvents(int nevents_to_process)
{
	// now randomizing the run of each event
	TextProgressBar bar(50, string(EVENTDISPENSERLOGMSGITEM) + " Distributing events according to run weights ", 0, nevents_to_process);

	// generating random number
	// reference: http://en.cppreference.com/w/cpp/numeric/random/uniform_real_distribution
	random_device randomDevice;
	mt19937 generator(randomDevice());
	uniform_real_distribution<> randomDistribution(0, 1);

	for(int i=0; i<nevents_to_process; i++) {
		double randomNumber = randomDistribution(generator);

		double ww = 0;
		for(const auto &weight : runWeights) {
			ww += weight.second;
			if(randomNumber <= ww) {
				runEvents[weight.first]++;
				break;
			}
		}
		bar.setProgress(i);
	}
}

// TODO: fix this as ntot is not used
void EventDispenser::printRunsDetails(int nevents_to_process)
{
	//	int ntot = 0;

	cout << EVENTDISPENSERLOGMSGITEM << " EventDispenser initialized with " << nevents_to_process << " events distributed among " << runWeights.size() << " runs:" << endl;

	if(verbosity >= GVERBOSITY_SUMMARY) {

		for(const auto &weight : runWeights) {
			cout << GTAB << EVENTDISPENSERLOGMSGITEM << " run: " << weight.first << "\t weight: " << runWeights[weight.first] ;
			cout << "\t  n. events: " << runEvents[weight.first] << endl;
			//			ntot += runEvents[weight.first];
		}
	}
}

// sums number of events across all runs
int EventDispenser::getTotalNumberOfEvents()
{
	int n = 0;

	for(auto rEvents : runEvents) {
		n += rEvents.second;
	}

	return n;
}


// this will:
// initiate all available gdynamic plugins for each run
// execute run/beamOn for each run
// log on screen infos if enough verbosity
int EventDispenser::processEvents()
{
	G4UImanager *g4uim = G4UImanager::GetUIpointer();
	g4uim->ApplyCommand("/run/printProgress " + to_string(elog));

	for(auto &run : runEvents) {

		int runNumber = run.first;
		int nevents   = run.second;

		// loads the constants
		if ( runNumber != currentRunno ) {
			for(auto [digitizationName, digiRoutine]: (*gDigitizationGlobal)) {
				if(verbosity >= GVERBOSITY_DETAILS) {
					gLogMessage(string(EVENTDISPENSERLOGMSGITEM) + " Calling " + string(KMAG) + digitizationName  +  string(RST) + " digitization loadConstants for run " + to_string(runNumber));
				}
				digiRoutine->loadConstants(runNumber, variation);
				if(verbosity >= GVERBOSITY_DETAILS) {
					gLogMessage(string(EVENTDISPENSERLOGMSGITEM) + " Calling " + string(KMAG) + digitizationName  +  string(RST) + " digitization loadTT for run " + to_string(runNumber));
				}
				digiRoutine->loadTT(runNumber, variation);
			}
			currentRunno = runNumber;
		}

		if(verbosity >= GVERBOSITY_SUMMARY) {
			gLogMessage(string(EVENTDISPENSERLOGMSGITEM) + " Starting " + string(KBLU) + " Run Number "  +  to_string(runNumber) + string(RST) + ", event buffer is: " + to_string(nEventBuffer));
		}

		if ( nevents <= nEventBuffer) {
			if(verbosity >= GVERBOSITY_SUMMARY) {
				gLogMessage("  " + string(EVENTDISPENSERLOGMSGITEM) + " Processing " + to_string(nevents) + " events");
			}
			g4uim->ApplyCommand("/run/initialize");
			g4uim->ApplyCommand("/run/beamOn " + to_string(nevents));
		} else {
			int nsubRuns = nevents / nEventBuffer ;
			int totalSoFar = 0;
			int lastSubRunNEvents = nevents%nEventBuffer;
			int ntotalSubRuns = ( lastSubRunNEvents > 0 ? nsubRuns + 1 : nsubRuns);
			for ( int s = 0; s < nsubRuns; s++ ) {
				if(verbosity >= GVERBOSITY_SUMMARY) {
                    // int upToNevents = totalSoFar + nEventBuffer;
					string log = "  " + string(EVENTDISPENSERLOGMSGITEM) + " Sub run: " + to_string(s+1) + "/" + to_string(ntotalSubRuns)
					+ ", processing events " + to_string(totalSoFar) + " → " + to_string(totalSoFar);
					gLogMessage(log);
				}
				g4uim->ApplyCommand("/run/initialize");
				g4uim->ApplyCommand("/run/beamOn " + to_string(nEventBuffer));
				totalSoFar += nEventBuffer;
			}
			if ( lastSubRunNEvents > 0 ) {
				if(verbosity >= GVERBOSITY_SUMMARY) {
					string log = "  " + string(EVENTDISPENSERLOGMSGITEM) + " Sub run: " + to_string(nsubRuns + 1) + "/" + to_string(nsubRuns + 1)
					                  + ", processing events " + to_string(totalSoFar) + " → " + to_string(lastSubRunNEvents);
					gLogMessage(log);
				}
				g4uim->ApplyCommand("/run/beamOn " + to_string(lastSubRunNEvents));
			}
		}

		if(verbosity >= GVERBOSITY_SUMMARY) {
			gLogMessage(string(EVENTDISPENSERLOGMSGITEM) + string(KBLU) + " Run Number "  +  to_string(runNumber) + string(RST) + " done with " + to_string(nevents) + " events" );
		}
	}

	return 1;
}


// show digitization constants and parameters
void EventDispenser::showDigitizationParameters(string system, vector<string> digiConstants, vector<string> digiPars)
{
	for(auto dc: digiConstants) {
		cout << "  " << EVENTDISPENSERLOGMSGITEM << " " << system << ": " << dc << endl;
	}
	for(auto dp: digiPars) {
		cout << "  " << EVENTDISPENSERLOGMSGITEM << " " << system << ": " << dp << endl;
	}
}
