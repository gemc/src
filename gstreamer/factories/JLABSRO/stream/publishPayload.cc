// gstreamer
#include "../gstreamerJLABSROFactory.h"
#include "gstreamerConventions.h"

bool GstreamerJSROFactory::publishPayloadImpl([[maybe_unused]] const std::vector<GIntegralPayload*>* payload) {
	if (ofile == nullptr) { log->error(ERR_CANTOPENOUTPUT, "Error: can't open ", ofile); }

	static constexpr int header_offset = sizeof(DataFrameHeader) / 4;
	std::vector<unsigned int> payload_data(frame_data.size() - header_offset);
	std::copy(frame_data.cbegin() + header_offset, frame_data.cend(), payload_data.begin());
	ofile->write(reinterpret_cast<const char*>(payload_data.data()),
	             sizeof(unsigned int) * payload_data.size());

	return true;
}
