// ghit
#include "ghit.h"

#include <utility>

// glibrary
#include "gutsConventions.h"

// geant4
#include "G4VVisManager.hh"
#include "G4Circle.hh"
#include "G4VisAttributes.hh"

using std::string;
using std::vector;

// MT definitions, as from:
// https://twiki.cern.ch/twiki/bin/view/Geant4/QuickMigrationGuideForGeant4V10
G4ThreadLocal G4Allocator<GHit>* GHitAllocator = nullptr;

GHit::GHit(GTouchable *gt, const HitBitSet hbs,  const G4Step *thisStep, string cScheme) : G4VHit(),
																						  colorSchema(std::move(cScheme)),
																						  gtouchable(gt) {

	// initialize quantities based on HitBitSet, like globalPositions
	if (thisStep) { addHitInfosForBitset(hbs, thisStep); }

	// unitialized quantities. To be calculated at the end of the steps by collectTrueInformation
	// bit 0: always there
	averageTime = UNINITIALIZEDNUMBERQUANTITY;
	avgGlobalPosition = G4ThreeVector(UNINITIALIZEDNUMBERQUANTITY, UNINITIALIZEDNUMBERQUANTITY, UNINITIALIZEDNUMBERQUANTITY);
	avgLocalPosition = G4ThreeVector(UNINITIALIZEDNUMBERQUANTITY, UNINITIALIZEDNUMBERQUANTITY, UNINITIALIZEDNUMBERQUANTITY);
	processName = UNINITIALIZEDSTRINGQUANTITY;

}

bool GHit::is_same_hit(GHit *hit) {
	return (*gtouchable) == (*hit->getGTouchable());
}


vector<int> GHit::getTTID() {
	vector<int> ttid;
	// Retrieve the identity vector from the associated GTouchable.
	vector <GIdentifier> gids = getGID();
	ttid.reserve(gids.size());
	for (auto &gid: gids) {
		// Push back the integer value of each identifier.
		ttid.push_back(gid.getValue());
	}
	return ttid;
}


/**
 * \brief Draws the hit using Geant4 visualization.
 *
 * Depending on the total energy deposited, selects different visual attributes.
 * Uses a G4Circle to represent the hit position. If the globalPositions vector is empty,
 * the method exits safely.
 */
 void GHit::Draw() {
	G4VVisManager *pVVisManager = G4VVisManager::GetConcreteInstance();

	// only care about schema if we are interactive
	if (pVVisManager) {
		setColorSchema();

		// Check that globalPositions is not empty before accessing the first element.
		if (globalPositions.empty()) return;

		G4Circle circle(globalPositions[0]);
		circle.SetFillStyle(G4Circle::filled);

		double etot = getTotalEnergyDeposited();

		if (etot > 0) {
			circle.SetScreenSize(10);
			circle.SetVisAttributes(G4VisAttributes(colour_hit));
		} else if (etot == 0) {
			circle.SetScreenSize(8);
			circle.SetVisAttributes(G4VisAttributes(colour_passby));
		}

		pVVisManager->Draw(circle);
	}
}


/**
 * \brief Sets the color schema for visualizing the hit.
 *
 * Chooses colors for hits with energy deposition versus pass-by events.
 * \return Currently always returns false.
 */
bool GHit::setColorSchema() {
	// For now, hard-code the color schema.
	colour_hit = G4Colour(1.0, 0.0, 0.0);      // Red for hits with energy.
	colour_passby = G4Colour(0.0, 1.0, 0.0);     // Green for pass-by.
	return false;
}