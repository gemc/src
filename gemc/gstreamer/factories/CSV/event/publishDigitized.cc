// gstreamer
#include "gstreamerCSVFactory.h"
#include "gstreamerConventions.h"

// Implementation summary:
// Flatten event-level digitized detector data into one CSV row per hit.

bool GstreamerCsvFactory::publishEventDigitizedDataImpl(const std::string&                        detectorName,
                                                        const std::vector<const GDigitizedData*>& digitizedData) {
	if (!ofile_digitized.is_open()) {
		log->error(ERR_CANTOPENOUTPUT, SFUNCTION_NAME, "Error: can't access ", filename_digitized());
	}

	// Emit the header row from the first non-empty detector collection so column names
	// reflect the actual observable schema.
	if (!is_first_event_with_digidata) {
		if (digitizedData.size() > 0) {
			ofile_digitized << "evn, timestamp, thread_id, detector, ";
			auto first_hit = digitizedData[0];

			const auto& imap = first_hit->getIntObservablesMap(0);
			const auto& dmap = first_hit->getDblObservablesMap(0);

			size_t total = dmap.size();
			size_t i     = 0;

			log->debug(NORMAL, SFUNCTION_NAME, "Writing header for run ", runId, " with ", total,
			           " variables");

			for (const auto& [name, value] : imap) { ofile_digitized << name << ", "; }
			for (const auto& [name, value] : dmap) {
				ofile_digitized << name;
				if (++i < total) ofile_digitized << ", ";
			}

			ofile_digitized << "\n";

			is_first_event_with_digidata = true;
		}
	}

	// Write one row per digitized hit.
	if (is_first_event_with_digidata) {
		for (auto digi_hit : digitizedData) {
			const auto& imap = digi_hit->getIntObservablesMap(0);
			const auto& dmap = digi_hit->getDblObservablesMap(0);

			size_t total = dmap.size();
			size_t i     = 0;

			ofile_digitized << event_number << ", " << timestamp << ", " << thread_id << ", " << detectorName << ", ";

			for (const auto& [variableName, value] : imap) { ofile_digitized << value << ", "; }
			for (const auto& [variableName, value] : dmap) {
				ofile_digitized << value;
				if (++i < total) ofile_digitized << ", ";
				else ofile_digitized << "\n";
			}
		}
	}

	return true;
}