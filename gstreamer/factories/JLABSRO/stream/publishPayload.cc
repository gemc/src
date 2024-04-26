// gstreamer
#include "../gstreamerJLABSROFactory.h"

bool GstreamerJSROFactory::publishPayload([[maybe_unused]] const vector<GIntegralPayload *> *payload) {
    if (ofile == nullptr) return false;

    static constexpr int header_offset = sizeof(DataFrameHeader) / 4;
    vector<unsigned int> payload_data(frame_data.size() - header_offset);
    std::copy(frame_data.cbegin() + header_offset, frame_data.cend(), payload_data.begin());
    ofile->write(reinterpret_cast<const char *>(payload_data.data()),
                 sizeof(unsigned int) * payload_data.size());

    return true;
}
