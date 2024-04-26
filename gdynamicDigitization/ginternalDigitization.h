#ifndef GINTERNALDIGITIZATION
#define GINTERNALDIGITIZATION 1

// glibrary
#include "gdynamicdigitization.h"
//#include "gutsConventions.h"

class GFluxDigitization : public GDynamicDigitization {

public:

	// mandatory readout specs definitions
	bool defineReadoutSpecs();

	// digitized the hit
	GDigitizedData* digitizeHit(GHit *ghit, size_t hitn);

};

class GParticleCounterDigitization : public GDynamicDigitization {

public:

	// mandatory readout specs definitions
	bool defineReadoutSpecs();

	// digitized the hit
	GDigitizedData* digitizeHit(GHit *ghit, size_t hitn);

};


class GDosimeterDigitization : public GDynamicDigitization {

public:

	// mandatory readout specs definitions
	bool defineReadoutSpecs();

	// digitized the hit
	GDigitizedData* digitizeHit(GHit *ghit, size_t hitn);

	// loads digitization constants
	bool loadConstants(int runno, string variation);


private:

	// key is particle id
	// currently:
	// 11 (electrons)
	// 211 (pions)
	// 2112 (neutrons)
	// 2212 (protons)
	map<int, vector<double> > nielfactorMap;
	map<int, vector<double> > E_nielfactorMap;

	map<int, double> pMassMeV;

	double getNielFactorForParticleAtEnergy(int pid, double energy);
};


#endif
