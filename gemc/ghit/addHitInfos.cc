// ghit
#include "ghit.h"

// glibrary
#include "gutsConventions.h"

// geant4
#include "G4VProcess.hh"

// See header for API docs.

void GHit::addHitInfos(const G4Step* step) {
	auto preStepPoint = step->GetPreStepPoint();

	auto touchable = preStepPoint->GetTouchable();

	// Global position and its local-coordinate transform.
	G4ThreeVector xyz  = preStepPoint->GetPosition();
	G4ThreeVector xyzL = touchable->GetHistory()->GetTopTransform().TransformPoint(xyz);

	globalPositions.push_back(xyz);
	localPositions.push_back(xyzL);

	// Energy deposition (scaled by detector multiplier) and global time.
	double edep = (step->GetTotalEnergyDeposit()) * (gtouchable->getEnergyMultiplier());
	double time = preStepPoint->GetGlobalTime();

	edeps.push_back(edep);
	times.push_back(time);

	auto track         = step->GetTrack();
	auto trackVertex   = track->GetVertexPosition();
	int  trackId       = track->GetTrackID();
	int  motherTrackId = track->GetParentID();
	int  currentPdg    = track->GetDefinition()->GetPDGEncoding();

	trackVertexById.emplace(trackId, trackVertex);
	pdgById.emplace(trackId, currentPdg);

	G4ThreeVector motherTrackVertex(UNINITIALIZEDNUMBERQUANTITY, UNINITIALIZEDNUMBERQUANTITY,
	                                UNINITIALIZEDNUMBERQUANTITY);
	int motherPdg = UNINITIALIZEDNUMBERQUANTITY;
	if (motherTrackId > 0) {
		auto motherVertex = trackVertexById.find(motherTrackId);
		if (motherVertex != trackVertexById.end()) {
			motherTrackVertex = motherVertex->second;
		}
		auto motherPdgIt = pdgById.find(motherTrackId);
		if (motherPdgIt != pdgById.end()) {
			motherPdg = motherPdgIt->second;
		}
	}

	trackVertexPositions.push_back(trackVertex);
	motherTrackVertexPositions.push_back(motherTrackVertex);
	pids.push_back(currentPdg);
	tids.push_back(trackId);
	motherTids.push_back(motherTrackId);
	momenta.push_back(preStepPoint->GetMomentum());
	trackEs.push_back(preStepPoint->GetTotalEnergy());
	motherPids.push_back(motherPdg);

	if (track->GetCreatorProcess()) {
		processNames.push_back(track->GetCreatorProcess()->GetProcessName());
	}
}
