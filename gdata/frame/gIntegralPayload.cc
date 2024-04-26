// gdata
#include "gIntegralPayload.h"


vector<int> GIntegralPayload::getPayload() {

	vector<int> payload;

	payload.push_back(crate);
	payload.push_back(slot);
	payload.push_back(channel);
	payload.push_back(charge);
	payload.push_back(time);

	return payload;
}
