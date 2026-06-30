#pragma once

// Geant4
#include "G4ThreeVector.hh"

// C++
#include <unordered_set>
#include <vector>

class G4Track;

/**
 * \brief Immutable initial information recorded for one Geant4 track.
 */
struct GTrackRecord
{
	int           pid = 0;
	int           tid = 0;
	int           mtid = 0;
	int           otid = 0;
	double        kinetic_energy = 0;
	G4ThreeVector momentum;
	G4ThreeVector vertex;
};

/**
 * \brief Worker-local, event-scoped track ancestry registry.
 *
 * Track IDs are used as vector indices. Original-track-only mode retains one integer per track;
 * full records are retained only when ancestor output is requested.
 */
class GTrackProvenance
{
public:
	explicit GTrackProvenance(bool save_ancestors);

	void clear();
	void record(const G4Track& track);
	void record(int pid, int track_id, int parent_id, double kinetic_energy,
	            const G4ThreeVector& momentum, const G4ThreeVector& vertex);

	[[nodiscard]] int originalTrackId(int track_id) const;
	[[nodiscard]] std::vector<GTrackRecord> ancestorsForTracks(
		const std::unordered_set<int>& track_ids) const;

private:
	void ensureCapacity(int track_id);

	bool                      save_ancestor_records = false;
	std::vector<int>          original_track_ids;
	std::vector<GTrackRecord> track_records;
};
