#ifndef  GINTEGRALPAYLOAD_H
#define  GINTEGRALPAYLOAD_H  1


// gemc
#include "glogger.h"

// c++
#include <iostream>


struct GIntegralPayload {
public:
	GIntegralPayload(int c, int s, int h, int q, int t, GLogger *const logger) :
			log(logger),
			crate(c),
			slot(s),
			channel(h),
			charge(q),
			time(t) {

		log->debug(CONSTRUCTOR, "GIntegralPayload crate ", crate, " slot ", slot, " channel ", channel, " charge ", charge, " time ", time);
	}

	~GIntegralPayload() {
		log->debug(DESTRUCTOR, "GIntegralPayload crate ", crate, " slot ", slot, " channel ", channel, " charge ", charge, " time ", time);
	}

	vector<int> getPayload()  {

		vector<int> payload;

		payload.push_back(crate);
		payload.push_back(slot);
		payload.push_back(channel);
		payload.push_back(charge);
		payload.push_back(time);

		return payload;
	}

private:
	GLogger *const log;

	int crate;
	int slot;
	int channel;
	int charge;
	int time;

};

#endif

