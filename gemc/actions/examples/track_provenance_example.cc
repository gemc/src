#include "tracking/gTrackProvenance.h"

// C++
#include <cstdlib>
#include <iostream>
#include <vector>

/**
 * \file track_provenance_example.cc
 * \brief Focused unit-test executable for the worker-local track provenance registry.
 *
 * This test uses synthetic records rather than running a Geant4 simulation. It builds this hierarchy:
 *
 * \code
 * 1
 * |-- 2
 * |   `-- 4
 * `-- 3
 * \endcode
 *
 * It verifies original-track resolution, ancestor deduplication and ordering, event reset, and the
 * memory-saving original-only mode. Real Geant4 integration is exercised by the GEMC example tests.
 */

namespace {
void add_track(GTrackProvenance& provenance, int tid, int mtid) {
	provenance.record(11, tid, mtid, 100.0 + tid, G4ThreeVector(tid, 2 * tid, 3 * tid),
	                  G4ThreeVector(4 * tid, 5 * tid, 6 * tid));
}
}

int main() {
	// Build the synthetic branching hierarchy documented above.
	GTrackProvenance provenance(true);
	add_track(provenance, 1, 0);
	add_track(provenance, 2, 1);
	add_track(provenance, 3, 1);
	add_track(provenance, 4, 2);

	// A primary is its own original track; every descendant resolves to that primary.
	if (provenance.originalTrackId(1) != 1 || provenance.originalTrackId(4) != 1) {
		std::cerr << "Incorrect original track ID\n";
		return EXIT_FAILURE;
	}

	// Request overlapping paths, including a duplicate, and require every track exactly once.
	const auto ancestors = provenance.ancestorsForTracks({4, 3, 4});
	if (ancestors.size() != 4) {
		std::cerr << "Expected four unique ancestor records, got " << ancestors.size() << '\n';
		return EXIT_FAILURE;
	}
	// Output is sorted by track ID so all streamers receive deterministic rows.
	for (std::size_t index = 0; index < ancestors.size(); ++index) {
		if (ancestors[index].tid != static_cast<int>(index + 1)) {
			std::cerr << "Ancestor records are not ordered by track ID\n";
			return EXIT_FAILURE;
		}
	}

	// Clearing at the next event boundary must remove both mappings and full records.
	provenance.clear();
	if (provenance.originalTrackId(4) != 0 || !provenance.ancestorsForTracks({4}).empty()) {
		std::cerr << "Event reset retained provenance\n";
		return EXIT_FAILURE;
	}

	// Original-only mode retains the compact ID mapping without allocating ancestor records.
	GTrackProvenance original_only(false);
	add_track(original_only, 1, 0);
	add_track(original_only, 2, 1);
	if (original_only.originalTrackId(2) != 1 || !original_only.ancestorsForTracks({2}).empty()) {
		std::cerr << "Original-only mode retained ancestor records\n";
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
