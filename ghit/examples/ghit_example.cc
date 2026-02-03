/**
 * \file ghit_example.cc
 * \brief Minimal example showing how to construct and compare \c GHit objects.
 *
 * This example demonstrates:
 * - Creating a \c GTouchable for a specific detector identity.
 * - Creating a \c GHit and storing it in a vector (emulating a \c G4THitsCollection<GHit> usage pattern).
 * - Generating randomized test hits via \c GHit::create() and comparing them using \c GHit::is_same_hit().
 *
 * \note This is an example program intended for demonstration and quick tests.
 */

// gemc
#include "gtouchable.h"
#include "gtouchable_options.h"
#include "glogger.h"

// ghit
#include "ghit.h"

using std::string;
using std::vector;

int main(int argc, char* argv[]) {
	// Build options using the touchable module option definitions.
	auto gopts = std::make_shared<GOptions>(argc, argv, gtouchable::defineOptions());

	// Local logger for this example program.
	auto log = std::make_shared<GLogger>(gopts, SFUNCTION_NAME, TOUCHABLE_LOGGER); // duplicate logger

	// Select which optional hit information to record.
	HitBitSet hitBitSet;

	// Create a concrete touchable corresponding to a detector element identity.
	auto a_ctof_gtouchable = GTouchable(gopts, "readout", "sector: 6, paddle: 10", {10.0, 20.0, 30.0});

	// Create a hit tied to that touchable. (No step is provided here, so vectors start empty.)
	auto a_hit = new GHit(std::make_shared<GTouchable>(a_ctof_gtouchable), hitBitSet);

	// Emulating a hits collection (in Geant4 this is commonly a G4THitsCollection<GHit>).
	vector<GHit*> hits;
	hits.emplace_back(a_hit); // pass ownership (caller owns and must delete if this were not a short-lived example)

	for (unsigned i = 1; i < 100; i++) {
		// Create randomized test hits (for demonstration only).
		auto hit = GHit::create(gopts);

		log->info(" GHit: ", *hit->getGTouchable(), " with total Edep: ", hit->getTotalEnergyDeposited(), " MeV");

		// Compare without copying; hits stores raw pointers, so iterate by pointer.
		for (const auto& hit_in_v : hits) {
			if (hit->is_same_hit(hit_in_v)) {
				log->info(NORMAL, (*hit->getGTouchable()), " found in hit n. ", i);
			}
		}

		// Note: in a real program you would also manage ownership of 'hit' to avoid leaks.
	}

	return EXIT_SUCCESS;
}
