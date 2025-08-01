// gemc
#include "gtouchable.h"
#include "gtouchable_options.h"
#include "glogger.h"

// ghit
#include "ghit.h"

using std::string;
using std::vector;

int main(int argc, char* argv[]) {
	auto gopts =std::make_shared<GOptions>(argc, argv, gtouchable::defineOptions());
	auto log   = std::make_shared<GLogger>(gopts, TOUCHABLE_LOGGER, "ghit_example");

	HitBitSet hitBitSet;

	auto a_ctof_gtouchable = GTouchable("readout", "sector: 6, paddle: 10", {10.0, 20.0, 30.0}, log);

	auto a_hit = new GHit(std::make_shared<GTouchable>(a_ctof_gtouchable), hitBitSet);

	// emulating GHitsCollection (which is G4THitsCollection<GHit>)
	vector<GHit*> hits;
	hits.emplace_back(a_hit); // pass ownership

	for (unsigned i = 1; i < 100; i++) {
		auto hit = GHit::create(log);

		log->info(" GHit: ", *hit->getGTouchable(), " with total Edep: ", hit->getTotalEnergyDeposited(), " MeV");

		// observing each std::unique_ptr<GHit> without copying it.
		for (const auto& hit_in_v : hits) { if (hit->is_same_hit(hit_in_v)) { log->info(NORMAL, (*hit->getGTouchable()), " found in hit n. ", i); } }
	}

	return EXIT_SUCCESS;
}
