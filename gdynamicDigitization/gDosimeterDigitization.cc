#include "ginternalDigitization.h"

// c++
using namespace std;

bool GDosimeterDigitization::defineReadoutSpecs() {
	float     timeWindow = 10;                  // electronic readout time-window of the detector
	float     gridStartTime = 0;                // defines the windows grid
	HitBitSet hitBitSet = HitBitSet("000001");  // defines what information to be stored in the hit
	bool      verbosity = true;

	readoutSpecs = new GReadoutSpecs(timeWindow, gridStartTime, hitBitSet, verbosity);

	return true;
}


// digitized the hit
GDigitizedData* GDosimeterDigitization::digitizeHit(GHit *ghit, size_t hitn) {

	// ghit->getGID() must have a single entry
	GIdentifier identity = ghit->getGID().front();

	GDigitizedData* gdata = new GDigitizedData(ghit);

	gdata->includeVariable(identity.getName(), identity.getValue()             );

	gdata->includeVariable("hitn",             (int) hitn                      );
	gdata->includeVariable("eTot",             ghit->getTotalEnergyDeposited() );
	gdata->includeVariable("time",             ghit->getAverageTime()          );

	auto pids = ghit->getPids();
	auto pEnergies = ghit->getEs();

	float nielWeight = 0;
	for( size_t stepIndex = 0; stepIndex < pids.size(); stepIndex++) {
		int pid = fabs(pids[stepIndex]); // absolute so we can catch -11 and -211

		if ( pid == 11 || pid == 211 || pid == 2212 || pid == 2112) {

			int E = pEnergies[stepIndex] - pMassMeV[pid];

			nielWeight += getNielFactorForParticleAtEnergy(pid, E);
		}
	}

	gdata->includeVariable("nielWeight", nielWeight);


	return gdata;
}

#include <fstream>
using std::ifstream;

// loads digitization constants
bool GDosimeterDigitization::loadConstants([[maybe_unused]] int runno, [[maybe_unused]] string variation) {

	// Niel Data
	// key is PID
	map<int, string> nielDataFiles;
	nielDataFiles[11]   = "niel_electron.txt";
	nielDataFiles[211]  = "niel_pion.txt";
	nielDataFiles[2112] = "niel_neutron.txt";
	nielDataFiles[2212] = "niel_proton.txt";

	// the data is loaded from the GPLUGIN_PATH location
	auto pluginPathENV = getenv("GPLUGIN_PATH"); // char*
	string pluginPath = UNINITIALIZEDSTRINGQUANTITY;

	if ( pluginPathENV != nullptr ) {
		pluginPath = string(pluginPathENV) + "/";
	}
	// set to current dir if pluginPath is still not defined
	if ( pluginPath == UNINITIALIZEDSTRINGQUANTITY ) {
		pluginPath = "./";
	}

	for ( auto [pid, filename]: nielDataFiles) {

		string dataFileWithPath = pluginPath + "/dosimeterData/Niel/" + filename;

		ifstream inputfile(dataFileWithPath);
		if(!inputfile) {
			cerr << " Error loading dosimeter data for pid <" << pid << "> from file " << dataFileWithPath  << endl;
			gexit(EC__FILENOTFOUND);
			return false;
		}

		cout << " Loading dosimeter data for pid <" << pid << "> from file " << filename <<  endl;
		double p0, p1;
		while( !inputfile.eof() ){

			inputfile >> p0 >> p1 ;

			nielfactorMap[pid].push_back(p0);
			E_nielfactorMap[pid].push_back(p1);
		}
		inputfile.close();
	}

	// load particle masses map
	pMassMeV[11]   = 0.510;
	pMassMeV[211]  = 139.570;
	pMassMeV[2112] = 939.565;
	pMassMeV[2212] = 938.272;


	return true;
}

double GDosimeterDigitization::getNielFactorForParticleAtEnergy(int pid, double energyMeV) {

	// input energy in MeV
	auto niel_N = nielfactorMap[pid].size();
	auto j = niel_N;

	for ( size_t i=0; i<niel_N; i++ ) {
		if ( energyMeV < E_nielfactorMap[pid][i] ) {
			j=i;
			break;
		}
	}


	double value;

	if (j>0 && j<niel_N) {
		auto nielfactorAtJ     = nielfactorMap[pid][j-1];
		auto nielfactorAtJM1   = nielfactorMap[pid][j];
		auto E_nielfactorAtJ   = E_nielfactorMap[pid][j-1];
		auto E_nielfactorAtJM1 = E_nielfactorMap[pid][j];

		value = nielfactorAtJM1 + ( nielfactorAtJ - nielfactorAtJM1 ) / ( E_nielfactorAtJ - E_nielfactorAtJM1 )*( energyMeV - E_nielfactorAtJM1);

	} else if (j==0) {
		value = nielfactorMap[pid].front();
	} else {
		value = nielfactorMap[pid].back();
	}

	// cout << " pid: " << pid << ", j: " << j << ", value: " << value << ", energy: " << energyMeV << endl;

	return value;
}
