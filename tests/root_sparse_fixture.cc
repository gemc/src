#include "gemc_options.h"
#include <gemc/gstreamer/gstreamer.h>
#include <gemc/gtouchable/gtouchable.h>

int main(int argc, char* argv[]) {
    auto options = std::make_shared<GOptions>(argc, argv, gemc::defineOptions());
    const bool collision = options->getSwitch("recordZeroEdep");
    auto touchable = GTouchable::create(options);
    GHit hit(touchable);
    // Deterministic sparse banks in two worker files: A hit/miss/hit, B late first hit.
    for (int thread = 0; thread < 2; ++thread) {
        auto streamers = gstreamer::gstreamersMapPtr(options, thread);
        for (auto& [name, streamer] : *streamers) streamer->openConnection();
        for (int event = 17; event <= 19; ++event) {
            auto data = std::make_shared<GEventDataCollection>(
                options, std::make_unique<GEventHeader>(options, event, thread));
            const std::string detector = event == 18 ? "B" : "A";
            auto truth = std::make_unique<GTrueInfoData>(options, &hit);
            truth->includeVariable("energy", 1.0);
            data->addDetectorTrueInfoData(detector, std::move(truth));
            // Reject A's first digitization while retaining its truth.
            if (event != 17) {
                auto digitized = std::make_unique<GDigitizedData>(options, &hit);
                digitized->includeVariable(collision ? "threadID" : "energy", 1.0);
                data->addDetectorDigitizedData(detector, std::move(digitized));
            }
            for (auto& [name, streamer] : *streamers) streamer->publishEventData(data);
        }
        for (auto& [name, streamer] : *streamers) streamer->closeConnection();
    }
    auto streamers = gstreamer::gstreamersMapPtr(options);
    auto run = std::make_shared<GRunDataCollection>(options, std::make_unique<GRunHeader>(options, 4));
    auto digitized = std::make_unique<GDigitizedData>(options, &hit);
    digitized->includeVariable("dose", 2.0);
    run->collect_event_data_collections("dose", std::move(digitized));
    for (auto& [name, streamer] : *streamers) {
        streamer->openConnection();
        streamer->publishRunData(run);
        streamer->closeConnection();
    }
}
