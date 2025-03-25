// gemc
#include "gtouchable.h"
#include "gtouchable_options.h"
#include "glogger.h"

// ghit
#include "ghit.h"

// c++
#include <iostream>

using namespace std;


int main(int argc, char *argv[]) {

	GOptions *gopts = new GOptions(argc, argv, gtouchable::defineOptions());
	GLogger *log = new GLogger(gopts, "GTouchable", "gtouchable");

	HitBitSet hitBitSet;

	string first_identity = "sector: 90, paddle: 90";
	vector<double> dimensions = {1.0, 20.0, 90.0};
	GTouchable *first_ctof = new GTouchable("readout", first_identity, dimensions, log);
	GHit *hit1 = new GHit(first_ctof, hitBitSet);

	// emulating GHitsCollection (which is G4THitsCollection<GHit>)
	vector<GHit *> hits;
	hits.push_back(hit1);


	for (unsigned i = 1; i < 100; i++) {
		string identity = "sector: " + to_string(i) + ", paddle: " + to_string(i);
		//GTouchable ctof("readout", identity, dimensions, log);
		GTouchable *ctof = new GTouchable("readout", identity, dimensions, log);
		GHit *hit = new GHit(ctof, hitBitSet);

		if (i % 10 == 0) {
			// loop over hits and check if this hit is the same as any in the vector
			for (auto &hit_in_v: hits) {
				if (hit->is_same_hit(hit_in_v)) {
					log->info(NORMAL, (*hit->getGTouchable()), " is the same as ", (*hit_in_v->getGTouchable()));
				} else {
					log->info(NORMAL, (*hit->getGTouchable()), " is not the same ", (*hit_in_v->getGTouchable()));
				}
			}
		}
	}




	return EXIT_SUCCESS;
}
