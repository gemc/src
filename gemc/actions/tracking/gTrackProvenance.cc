#include "gTrackProvenance.h"

// Geant4
#include "G4ParticleDefinition.hh"
#include "G4Track.hh"

// C++
#include <cstddef>

GTrackProvenance::GTrackProvenance(bool save_ancestors) : save_ancestor_records(save_ancestors) {
	clear();
}

void GTrackProvenance::clear() {
	original_track_ids.assign(1, 0);
	original_track_pids.assign(1, 0);
	original_track_momenta.assign(1, G4ThreeVector());
	track_records.clear();
	if (save_ancestor_records) { track_records.resize(1); }
}

void GTrackProvenance::ensureCapacity(int track_id) {
	if (track_id < 0) { return; }
	const auto required_size = static_cast<std::size_t>(track_id) + 1;
	if (original_track_ids.size() < required_size) { original_track_ids.resize(required_size, 0); }
	if (original_track_pids.size() < required_size) { original_track_pids.resize(required_size, 0); }
	if (original_track_momenta.size() < required_size) {
		original_track_momenta.resize(required_size, G4ThreeVector());
	}
	if (save_ancestor_records && track_records.size() < required_size) {
		track_records.resize(required_size);
	}
}

void GTrackProvenance::record(const G4Track& track) {
	record(track.GetParticleDefinition()->GetPDGEncoding(), track.GetTrackID(), track.GetParentID(),
	       track.GetKineticEnergy(), track.GetMomentum(), track.GetVertexPosition());
}

void GTrackProvenance::record(int pid, int track_id, int parent_id, double kinetic_energy,
	                          const G4ThreeVector& momentum, const G4ThreeVector& vertex) {
	if (track_id <= 0) { return; }

	ensureCapacity(track_id);

	const int           original_id       = parent_id == 0 ? track_id : originalTrackId(parent_id);
	const int           original_pid      = parent_id == 0 ? pid : originalTrackPid(parent_id);
	const G4ThreeVector original_momentum = parent_id == 0 ? momentum : originalTrackMomentum(parent_id);
	original_track_ids[track_id]     = original_id;
	original_track_pids[track_id]    = original_pid;
	original_track_momenta[track_id] = original_momentum;

	if (!save_ancestor_records) { return; }

	track_records[track_id] = {
		pid,
		track_id,
		parent_id,
		original_id,
		kinetic_energy,
		momentum,
		vertex
	};
}

int GTrackProvenance::originalTrackId(int track_id) const {
	if (track_id <= 0 || static_cast<std::size_t>(track_id) >= original_track_ids.size()) { return 0; }
	return original_track_ids[track_id];
}

int GTrackProvenance::originalTrackPid(int track_id) const {
	if (track_id <= 0 || static_cast<std::size_t>(track_id) >= original_track_pids.size()) { return 0; }
	return original_track_pids[track_id];
}

G4ThreeVector GTrackProvenance::originalTrackMomentum(int track_id) const {
	if (track_id <= 0 || static_cast<std::size_t>(track_id) >= original_track_momenta.size()) {
		return G4ThreeVector();
	}
	return original_track_momenta[track_id];
}

std::vector<GTrackRecord> GTrackProvenance::ancestorsForTracks(
	const std::unordered_set<int>& track_ids) const {
	std::vector<GTrackRecord> ancestors;
	if (!save_ancestor_records) { return ancestors; }

	std::vector<bool> selected(track_records.size(), false);
	std::size_t selected_count = 0;
	for (int track_id : track_ids) {
		while (track_id > 0 && static_cast<std::size_t>(track_id) < track_records.size() && !selected[track_id]) {
			const auto& record = track_records[track_id];
			if (record.tid == 0) { break; }
			selected[track_id] = true;
			++selected_count;
			track_id = record.mtid;
		}
	}

	ancestors.reserve(selected_count);
	for (std::size_t track_id = 1; track_id < track_records.size(); ++track_id) {
		if (selected[track_id]) { ancestors.push_back(track_records[track_id]); }
	}
	return ancestors;
}
