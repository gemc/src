// ghit
#include "ghit.h"

// glibrary
#include "gutsConventions.h"

// geant4
#include "G4VVisManager.hh"
#include "G4Circle.hh"
#include "G4VisAttributes.hh"

// MT definitions, as from:
// https://twiki.cern.ch/twiki/bin/view/Geant4/QuickMigrationGuideForGeant4V10
G4ThreadLocal G4Allocator<GHit>* GHitAllocator = 0;

GHit::GHit(GTouchable *gt, const G4Step* thisStep, const HitBitSet hbs, string cScheme) : G4VHit(),
colorSchema(cScheme),
gtouchable(gt) {

	// initialize quantities based on HitBitSet, like globalPositions
	addHitInfosForBitset(hbs, thisStep);

	// unitialized quantities. To be calculated at the end of the steps by collectTrueInformation
	// bit 0: always there
	totalEnergyDeposited = UNINITIALIZEDNUMBERQUANTITY;
	averageTime          = UNINITIALIZEDNUMBERQUANTITY;
	avgGlobalPosition = G4ThreeVector(UNINITIALIZEDNUMBERQUANTITY, UNINITIALIZEDNUMBERQUANTITY, UNINITIALIZEDNUMBERQUANTITY);
	avgLocalPosition  = G4ThreeVector(UNINITIALIZEDNUMBERQUANTITY, UNINITIALIZEDNUMBERQUANTITY, UNINITIALIZEDNUMBERQUANTITY);
	processName       = UNINITIALIZEDSTRINGQUANTITY;

}

GHit::~GHit() {
	
}


vector<int> GHit::getTTID() {
	vector<int> ttid;

	vector<GIdentifier> gids = getGID();

	for ( auto& gid : gids ) {
		ttid.push_back(gid.getValue());
	}

	return ttid;
}

void GHit::Draw()
{
	G4VVisManager* pVVisManager = G4VVisManager::GetConcreteInstance();

	// only care about schema if we are interactie
	if(pVVisManager) {
		setColorSchema();
		
		G4Circle circle(globalPositions[0]);
		circle.SetFillStyle(G4Circle::filled);

		double etot = getTotalEnergyDeposited();

		if( etot > 0 ) {
			circle.SetScreenSize(10);
			circle.SetVisAttributes(G4VisAttributes(colour_hit));
		} else if( etot == 0) {
			circle.SetScreenSize(8);
			circle.SetVisAttributes(G4VisAttributes(colour_passby));
		}
		
		pVVisManager->Draw(circle);

	}
}


// sets marker type, size, open or filled, its color based on its energy deposited
// notice: colorSchema should drive this
bool GHit::setColorSchema()
{
	colour_hit    = G4Colour(1.0, 0.0, 0.0);
	colour_passby = G4Colour(0.0, 1.0, 0.0);

	return false;
}

