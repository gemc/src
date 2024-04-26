#ifndef  GFRAMEDATA_H
#define  GFRAMEDATA_H  1

// gdata
#include "gFrameDataCollectionHeader.h"
#include "gIntegralPayload.h"
#include "../gdataConventions.h"

// c++
#include <vector>
using std::vector;

class GFrameDataCollection
{
public:
	// construct event data using a GEventHeader
	GFrameDataCollection(GFrameDataCollectionHeader* header, int v = 0 ) : verbosity(v), gheader(header) {
		if ( verbosity >= GVERBOSITY_CLASSES) {
			gLogClassConstruct("GFrameDataCollection");
		}
		integralPayloads = new vector<GIntegralPayload*>();
	}

	~GFrameDataCollection() {

		if ( verbosity >= GVERBOSITY_CLASSES) {
			gLogClassDestruct("GFrameDataCollection");
		}

		delete gheader;

		for ( auto* payload: *integralPayloads )  { delete payload; }
		delete integralPayloads;
	}

public:

	// integral payload: integrated quantity
	void addIntegralPayload(vector<int> payload, int verbosity);
	void addEvent(int evn);
	bool shouldWriteFrame() const;

	// getters
	inline const GFrameDataCollectionHeader* getHeader()         const { return gheader; }
	inline const vector<GIntegralPayload*>* getIntegralPayload() const { return integralPayloads; }
	inline long int getFrameID()                           const { return gheader->getFrameID(); }

private:
	int verbosity;

	GFrameDataCollectionHeader *gheader = nullptr;

	vector<GIntegralPayload*> *integralPayloads;


};


#endif
