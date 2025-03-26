#ifndef  GFRAMEDATA_H
#define  GFRAMEDATA_H  1

// gdata
#include "gFrameDataCollectionHeader.h"
#include "gIntegralPayload.h"
#include "../gdataConventions.h"

// c++
#include <vector>

class GFrameDataCollection {
public:
	// construct event data using a GEventHeader
	GFrameDataCollection(GFrameDataCollectionHeader *header, GLogger *const logger) : gheader(header), log(logger) {
		log->debug(CONSTRUCTOR, "GFrameDataCollection");
		integralPayloads = new vector<GIntegralPayload *>();
	}

	~GFrameDataCollection() {
		log->debug(DESTRUCTOR, "GFrameDataCollection");

		delete gheader;

		for (auto *payload: *integralPayloads) { delete payload; }
		delete integralPayloads;
	}

public:

	// integral payload: integrated quantity
	void addIntegralPayload(vector<int> payload)  {

		if (payload.size() == 5 ) {
			int crate   = payload[0];
			int slot    = payload[1];
			int channel = payload[2];
			int charge  = payload[3];
			int time    = payload[4];

			GIntegralPayload *gpayload = new GIntegralPayload(crate, slot, channel, charge, time, v);
			integralPayloads->push_back(gpayload);

			log->debug(NORMAL, " adding integral payload for crate ", crate, " slot ", slot, " channel ", channel, " charge ", charge, " time ", time);

		} else {
			log->error(EC__WRONGPAYLOAD, "payload size is not 4 but ", payload.size());
		}

	}

	void addEvent(int evn);

	bool shouldWriteFrame() const;

	// getters
	inline const GFrameDataCollectionHeader *getHeader() const { return gheader; }

	inline const vector<GIntegralPayload *> *getIntegralPayload() const { return integralPayloads; }

	inline long int getFrameID() const { return gheader->getFrameID(); }

private:
	GLogger *const log;

	GFrameDataCollectionHeader *gheader = nullptr;

	vector<GIntegralPayload *> *integralPayloads;


};


#endif
