// gstreamer
#include "../gstreamerASCIIFactory.h"
#include "gstreamerConventions.h"

bool GstreamerTextFactory::publishPayloadImpl(const std::vector<GIntegralPayload*> *payload)
{
	if (ofile == nullptr) { log->error(ERR_CANTOPENOUTPUT, "Error: can't open ", ofile); }

	*ofile << GTAB << "Payload  {" << std::endl;

	for ( auto pload: *payload ) {

		for (auto pvar: pload->getPayload() ) {
			*ofile << GTABTAB << pvar ;
		}
		*ofile << std::endl;

	}
	*ofile << GTAB << "}" << std::endl;

	return true;
}
