// gstreamer
#include "gstreamerCSVFactory.h"
#include "gstreamerConventions.h"

// using \n instead of endl so flushing isn't forced at each line
// Non-Doxygen implementation file: behavior is documented in the header.
bool GstreamerCsvFactory::publishEventTrueInfoDataImpl(const std::string&                       detectorName,
                                                       const std::vector<const GTrueInfoData*>& trueInfoData) {
	if (!ofile_true_info.is_open()) {
		log->error(ERR_CANTOPENOUTPUT, SFUNCTION_NAME, "Error: can't access ", filename_true_info());
	}

	// First non-empty event: print header from the first hit so columns match the hit variable maps.
	if (!is_first_event_with_truedata) {
		if (trueInfoData.size() > 0) {
			ofile_true_info << "evn, timestamp, thread_id, detector, ";
			auto first_hit = trueInfoData[0];

			const auto& smap = first_hit->getStringVariablesMap();
			const auto& dmap = first_hit->getDoubleVariablesMap();

			size_t total = dmap.size();
			size_t i     = 0;

			log->debug(NORMAL, SFUNCTION_NAME, "Writing header for event ", event_number, " with ", total,
			           " variables");

			for (const auto& [name, value] : smap) { ofile_true_info << name << ", "; }

			for (const auto& [name, value] : dmap) {
				ofile_true_info << name;
				if (++i < total) ofile_true_info << ", "; // not last, write comma
			}

			ofile_true_info << "\n";

			is_first_event_with_truedata = true;
		}
	}

	// If we emitted a header, we have a stable column set and can write rows.
	if (is_first_event_with_truedata) {
		for (auto trueInfoHit : trueInfoData) {
			const auto& smap = trueInfoHit->getStringVariablesMap();
			const auto& dmap = trueInfoHit->getDoubleVariablesMap();

			size_t total = dmap.size();
			size_t i     = 0;

			ofile_true_info << event_number << ", " << timestamp << ", " << thread_id << ", " << detectorName << ", ";

			for (const auto& [variableName, value] : smap) { ofile_true_info << value << ", "; }
			for (const auto& [variableName, value] : dmap) {
				ofile_true_info << value;
				if (++i < total) ofile_true_info << ", "; // not last, write comma
				else ofile_true_info << "\n";
			}
		}
	}

	return true;
}
