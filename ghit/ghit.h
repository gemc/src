#ifndef  GHIT_H
#define  GHIT_H 1

// This example is based on B4cCalorHit in  examples/basic/B4/B4c
// find with locate examples/basic/B4/B4c

// HitBitSet definition
#include "ghitConventions.h"

// geant4
#include "G4VHit.hh"
#include "G4THitsCollection.hh"
#include "G4Allocator.hh"
#include "G4ThreeVector.hh"
#include "G4Step.hh"
#include "G4Colour.hh"

// glibrary
#include "gtouchable.h"

class GHit : public G4VHit
{
public:
	GHit(GTouchable *gt, const G4Step* thisStep, const HitBitSet hbs, const string cScheme = "default");

	virtual ~GHit();

	// examples/basic/B4/B4c/include/CalorHit
	inline void* operator new(size_t);
	inline void  operator delete(void*);
	// Overloaded "==" operator for the class 'GHit'
	// bool operator== (const GHit& that) const;

	// G4VHit: draws an object at hit location
	void Draw();

private:

	G4Colour colour_touch, colour_hit, colour_passby;
	bool setColorSchema();
	string colorSchema;

	// GTouchable saved here so it can be used in the overloaded == function
	GTouchable *gtouchable;

	// hit data, selected by HitBitSet, to be collected for each step
	// always present:
	vector<float> edeps, times;
	vector<G4ThreeVector> globalPositions;
	vector<G4ThreeVector> localPositions;

	// bit 1
	vector<int> pids;
	vector<float> Es;
	vector<string> processNames;

	// bit 2
	vector<float> stepSize;

	// calculated/single quantities
	// initialized to UNINITIALIZEDNUMBERQUANTITY. Set/retrieved with methods below
	float totalEnergyDeposited;
	float averageTime;
	G4ThreeVector avgGlobalPosition;
	G4ThreeVector avgLocalPosition;
	string processName;

	// build hit information based on the bit index and the touchable
	bool addHitInfosForBitIndex(size_t bitIndex, const bool test, const G4Step* thisStep);

public:
	// public interface: getting step by step quantities
	inline const vector<float> getEdeps()                   const { return edeps;}
	inline const vector<float> getTimes()                   const { return times;}
	inline const vector<G4ThreeVector> getGlobalPositions() const { return globalPositions;}
	inline const vector<G4ThreeVector> getLocalPositions()  const { return localPositions; }
	inline const vector<int> getPids()                      const { return pids;}
	inline int getPid()                               const { return pids.front();}
	inline vector<float> getEs()                      const { return Es;}
	inline float getE()                               const { return Es.front();}
	inline const string getProcessName()                    const { return processName; }

	// calculated quantities
	void calculateInfosForBit(int bit);
	float getTotalEnergyDeposited();
	float getAverageTime();
	G4ThreeVector getAvgLocalPosition();
	G4ThreeVector getAvgGlobaPosition();

	// gemc api
	// build hit information based on the G4Step
	void addHitInfosForBitset(const HitBitSet hbs, const G4Step* thisStep);

	inline const GTouchable*         getGTouchable()         const { return gtouchable; }
	inline const vector<GIdentifier> getGID()                const { return gtouchable->getIdentity(); }
	inline const vector<double>      getDetectorDimensions() const { return gtouchable->getDetectorDimensions(); }

	// returns gtouchable identity values
	vector<int> getTTID();

};

// MT definitions, as from:
// https://twiki.cern.ch/twiki/bin/view/Geant4/QuickMigrationGuideForGeant4V10
extern G4ThreadLocal G4Allocator<GHit>* GHitAllocator;

using GHitsCollection = G4THitsCollection<GHit> ;

inline void* GHit::operator new(size_t)
{
	if(!GHitAllocator) GHitAllocator = new G4Allocator<GHit>;
	return (void *) GHitAllocator->MallocSingle();

}

inline void GHit::operator delete(void *hit)
{
	if (!GHitAllocator) {
		GHitAllocator = new G4Allocator<GHit>;
	}

	GHitAllocator->FreeSingle((GHit*) hit);
}




#endif
