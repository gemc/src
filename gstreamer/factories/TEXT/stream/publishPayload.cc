// gstreamer
#include "../gstreamerTEXTFactory.h"

bool GstreamerTextFactory::publishPayload(const vector<GIntegralPayload*> *payload)
{
	if(ofile == nullptr) return false;


	*ofile << GTAB << "Payload  {" << endl;

	for ( auto pload: *payload ) {

		for (auto pvar: pload->getPayload() ) {
			*ofile << GTABTAB << pvar ;
		}
		*ofile << endl;

	}
	*ofile << GTAB << "}" << endl;

	return true;
}
