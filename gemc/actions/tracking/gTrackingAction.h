#pragma once

// Geant4
#include "G4UserTrackingAction.hh"

// C++
#include <memory>

class GTrackProvenance;

/**
 * \brief Records initial track provenance for one Geant4 worker.
 */
class GTrackingAction : public G4UserTrackingAction
{
public:
	explicit GTrackingAction(std::shared_ptr<GTrackProvenance> provenance);

	void PreUserTrackingAction(const G4Track* track) override;

private:
	std::shared_ptr<GTrackProvenance> track_provenance;
};
