// ghit
#include "ghit.h"

// glibrary
#include "gutsConventions.h"

// geant4
#include "G4VVisManager.hh"
#include "G4Circle.hh"
#include "G4VisAttributes.hh"
#include "Randomize.hh"

// c++
#include <algorithm>
#include <set>

using std::string;
using std::vector;

namespace {

constexpr int opticalPhotonPid = -22; // Geant4 optical-photon PDG encoding.

} // namespace

std::atomic<int> GHit::globalHitCounter{0};
thread_local std::map<int, G4ThreeVector> GHit::trackVertexById;
thread_local std::map<int, int> GHit::pdgById;

// MT definitions, as from:
// https://twiki.cern.ch/twiki/bin/view/Geant4/QuickMigrationGuideForGeant4V10
G4ThreadLocal G4Allocator<GHit>* GHitAllocator = nullptr;

// See header for API docs.

GHit::GHit(std::shared_ptr<GTouchable> gt,
           const G4Step*               thisStep,
           const string&               cScheme) :
	G4VHit(),
	colorSchema(cScheme),
	gtouchable(gt) {
	// Initialize per-step vectors if a step is provided.
	if (thisStep) { addHitInfos(thisStep); }
}

bool GHit::is_same_hit(const GHit* hit) const {
	if (!hit) // guard against nullptr
		return false;

	return *gtouchable == *(hit->getGTouchable());
}

vector<int> GHit::getTTID() const {
	vector<int> ttid;
	// Retrieve the identity vector from the associated GTouchable.
	vector<GIdentifier> gids = getGID();
	ttid.reserve(gids.size());
	for (auto& gid : gids) {
		// Push back the integer value of each identifier.
		ttid.push_back(gid.getValue());
	}
	return ttid;
}

void GHit::Draw() {
	auto visManager = G4VVisManager::GetConcreteInstance();
	if (!visManager) return;

	// Only care about schema if we are interactive.
	setColorSchema();

	// Check that globalPositions is not empty before accessing the first element.
	if (globalPositions.empty()) return;

	G4Circle circle(globalPositions[0]);
	circle.SetFillStyle(G4Circle::filled);

	double etot = getTotalEnergyDeposited();
	const bool opticalPhotonHit = !pids.empty() && pids.front() == opticalPhotonPid;

	if (opticalPhotonHit) {
		circle.SetScreenSize(15);
		circle.SetVisAttributes(G4VisAttributes(colour_passby));
	}
	else if (etot > 0) {
		circle.SetScreenSize(50);
		circle.SetVisAttributes(G4VisAttributes(colour_hit));
	}
	else if (etot == 0) {
		circle.SetScreenSize(15);
		circle.SetVisAttributes(G4VisAttributes(colour_passby));
		circle.SetFillStyle(G4Circle::hashed);
	}

	visManager->Draw(circle);
}

size_t GHit::getNumberOfOpticalPhotons() const {
	std::set<int> photonTrackIds;
	const size_t entries = std::min(pids.size(), tids.size());

	for (size_t index = 0; index < entries; ++index) {
		if (pids[index] == opticalPhotonPid) { photonTrackIds.insert(tids[index]); }
	}

	return photonTrackIds.size();
}

bool GHit::setColorSchema() {
	// For now, hard-code the color schema.
	colour_hit    = G4Colour(1.0, 0.0, 0.0); // Red for hits with energy.
	colour_passby = G4Colour(0.0, 1.0, 0.0); // Green for pass-by.
	return false;
}

void GHit::clearTrackVertexCache() {
	trackVertexById.clear();
	pdgById.clear();
}

void GHit::randomizeHitForTesting(int nsteps) {
	// This function is for testing purposes only.
	// It randomizes the hit's global position and energy deposition.
	// It should not be used in production code.

	invalidateCalculatedState();

	// Generate nsteps+1 entries to preserve the existing behavior exactly.
	for (int i = 0; i < nsteps + 1; ++i) {
		globalPositions.emplace_back(G4UniformRand() * 100, G4UniformRand() * 100, G4UniformRand() * 100);
		localPositions.emplace_back(G4UniformRand() * 10, G4UniformRand() * 10, G4UniformRand() * 10);
		trackVertexPositions.emplace_back(G4UniformRand() * 100, G4UniformRand() * 100, G4UniformRand() * 100);
		motherInfos.push_back({0, std::nullopt, std::nullopt});
		times.emplace_back(G4UniformRand() * 100);
		edeps.emplace_back(G4UniformRand() * 10);
		pids.emplace_back(11);
		tids.emplace_back(i);
		momenta.emplace_back(G4UniformRand() * 100, G4UniformRand() * 100, G4UniformRand() * 100);
		trackEs.emplace_back(G4UniformRand() * 1000);
		processNames.emplace_back("placeholder");
	}
}
