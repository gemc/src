// ghit
#include "ghit.h"

// geant4
#include "G4VProcess.hh"

// See header for API docs.

void GHit::addHitInfos(const G4Step* step) {
	invalidateCalculatedState();

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

	MotherInfo motherInfo{motherTrackId, std::nullopt, std::nullopt};
	if (motherTrackId > 0) {
		auto motherVertex = trackVertexById.find(motherTrackId);
		if (motherVertex != trackVertexById.end()) {
			motherInfo.vertex = motherVertex->second;
		}
		auto motherPdgIt = pdgById.find(motherTrackId);
		if (motherPdgIt != pdgById.end()) {
			motherInfo.pid = motherPdgIt->second;
		}
	}

	trackVertexPositions.push_back(trackVertex);
	motherInfos.push_back(std::move(motherInfo));
	pids.push_back(currentPdg);
	tids.push_back(trackId);
	momenta.push_back(preStepPoint->GetMomentum());
	trackEs.push_back(preStepPoint->GetTotalEnergy());

	if (track->GetCreatorProcess()) {
		processNames.push_back(track->GetCreatorProcess()->GetProcessName());
	}
}
