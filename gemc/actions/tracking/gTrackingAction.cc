#include "gTrackingAction.h"

#include "gTrackProvenance.h"

// C++
#include <utility>

GTrackingAction::GTrackingAction(std::shared_ptr<GTrackProvenance> provenance) :
	track_provenance(std::move(provenance)) {
}

void GTrackingAction::PreUserTrackingAction(const G4Track* track) {
	if (track_provenance != nullptr && track != nullptr) { track_provenance->record(*track); }
}
