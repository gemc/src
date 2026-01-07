/**
* \file gEunDataCollection.cc
 * \brief Implementation of the GRunDataCollection class.
 */

#include "gRunDataCollection.h"

void GRunDataCollection::collectDetectorTrueInfoData(const std::string& sdName, const std::unique_ptr<GTrueInfoData>& data) {
    if (gdataCollectionMap.find(sdName) == gdataCollectionMap.end()) {
        gdataCollectionMap[sdName] = std::make_unique<GDataCollection>();
    }
    gdataCollectionMap[sdName]->collectTrueInfosData(std::make_unique<GTrueInfoData>(*data));
    log->info(2, "GRunDataCollection: collected detector TrueInfoData for ", sdName);
}

void GRunDataCollection::collectDetectorDigitizedData(const std::string& sdName, const std::unique_ptr<GDigitizedData>& data) {
    if (gdataCollectionMap.find(sdName) == gdataCollectionMap.end()) {
        gdataCollectionMap[sdName] = std::make_unique<GDataCollection>();
    }
    gdataCollectionMap[sdName]->collectDigitizedData(std::make_unique<GDigitizedData>(*data));
    log->info(2, "GRunDataCollection: collected  detector DigitizedData for ", sdName);
}

void  GRunDataCollection::collect_event_data_collection(const std::shared_ptr<GEventDataCollection> edc) {

    // get the data collection map
    auto & dcm = edc->getDataCollectionMap();

    // loop over sensitive detectors maps
    for (auto & [sdname, ptr] : dcm) {  // note all '&'
        if (!ptr) continue;
        // use *ptr
        auto & true_infos_data = ptr->getTrueInfoData();
        for ( auto & true_info_hit : true_infos_data) {
            collectDetectorTrueInfoData(sdname, true_info_hit);
        }

        auto & digitized_data  = ptr->getDigitizedData();
        for ( auto & digitized_data_hit : digitized_data) {
            collectDetectorDigitizedData(sdname, digitized_data_hit);
        }
    }

}
