// gstreamer
#include "../gstreamerJLABSROFactory.h"

bool GstreamerJSROFactory::startStream(const GFrameDataCollection *frameRunData) {

    if (ofile == nullptr) {
        cout << "ofile == nullptr" << endl;
        return false;
    }

    static constexpr int header_offset = sizeof(DataFrameHeader) / 4;
    const GFrameDataCollectionHeader *header = frameRunData->getHeader();
    long int frameID = header->getFrameID();
    const std::vector<GIntegralPayload *> *intPayloadvec = frameRunData->getIntegralPayload();

    frame_data.resize(header_offset, 0);

    if (frameID == 1) {
        std::vector<std::uint32_t> const super_magic = {0xC0DA2019, 0XC0DA0001};
        ofile->write(reinterpret_cast<const char *>(super_magic.data()), sizeof(std::uint32_t) * 2);
    }

    // fill dataFrameHeader here or in publishFrameHeader
    DataFrameHeader &dataFrameHeader = *reinterpret_cast<DataFrameHeader *>(frame_data.data());

    dataFrameHeader.source_id = 0;
    dataFrameHeader.magic = 0xC0DA2019;
    dataFrameHeader.format_version = 257;
    dataFrameHeader.flags = 0;
    dataFrameHeader.record_counter = llswap(frameID);
    dataFrameHeader.ts_sec = llswap((frameID * 65536) / static_cast<int>(1e9));
    dataFrameHeader.ts_nsec = llswap((frameID * 65536) % static_cast<int>(1e9));


    //make payload data
    unsigned int crate = 0;
    unsigned int slot;
    unsigned int channel;
    unsigned int charge;
    unsigned int time;
    unsigned int slots = 16;

    frame_data.resize(header_offset);
    frame_data.push_back(0x80000000);
    frame_data.insert(frame_data.end(), slots, 0);

    for (unsigned int i = 0; i < slots; ++i) {

        int starting_point = (int) frame_data.size() - header_offset;
        frame_data.push_back(0x80008000 | (crate << 8) | i);
        int hit_counter = 0;

        for (unsigned int hit = 0; hit < intPayloadvec->size(); ++hit) {

            GIntegralPayload *intpayload = intPayloadvec->at(hit);
            vector<int> payload = intpayload->getPayload();
            crate = payload[0];
            slot = payload[1];
            channel = payload[2];
            charge = payload[3];
            time = payload[4];

            if (i == slot) {
                //	      if(hit<100) cout << time << endl;
                frame_data.push_back(charge | (channel << 13) | ((time / 4) << 17));
                ++hit_counter;
            }
        }

        if (hit_counter == 0) {
            frame_data.pop_back();
        } else {
            ++hit_counter;
        }

        frame_data[header_offset + 1 + i] =
                ((hit_counter) << 16) | starting_point;
    }

    DataFrameHeader &dfh = *reinterpret_cast<DataFrameHeader *>(frame_data.data());

    dfh.payload_length = (uint32_t) frame_data.size() * sizeof(unsigned int) - sizeof(DataFrameHeader);
    dfh.compressed_length = dfh.payload_length;
    dfh.total_length = dfh.compressed_length + sizeof(DataFrameHeader) - 4;

    return true;
}


bool GstreamerJSROFactory::endStream([[maybe_unused]] const GFrameDataCollection *frameRunData) {
    if (ofile == nullptr) return false;


    return true;
}
