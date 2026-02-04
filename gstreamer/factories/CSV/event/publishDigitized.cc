// gstreamer
#include "gstreamerCSVFactory.h"
#include "gstreamerConventions.h"

// using \n instead of endl so flushing isn't forced at each line
// Non-Doxygen implementation file: behavior is documented in the header.
bool GstreamerCsvFactory::publishEventDigitizedDataImpl(const std::string&                        detectorName,
                                                        const std::vector<const GDigitizedData*>& digitizedData) {
	if (!ofile_digitized.is_open()) {
		log->error(ERR_CANTOPENOUTPUT, SFUNCTION_NAME, "Error: can't access ", filename_digitized());
	}

	// First non-empty event: print header from the first hit so columns match the hit variable maps.
	if (!is_first_event_with_digidata) {
		if (digitizedData.size() > 0) {
			ofile_digitized << "evn, timestamp, thread_id, detector, ";
			auto first_hit = digitizedData[0];

			// Argument passed to getter: 0 = do not get sro vars.
			const auto& imap = first_hit->getIntObservablesMap(0);
			const auto& dmap = first_hit->getDblObservablesMap(0);

			size_t total = dmap.size();
			size_t i     = 0;

			log->debug(NORMAL, SFUNCTION_NAME, "Writing header for event ", event_number, " with ", total,
			           " variables");

			for (const auto& [name, value] : imap) { ofile_digitized << name << ", "; }

			for (const auto& [name, value] : dmap) {
				ofile_digitized << name;
				if (++i < total) ofile_digitized << ", "; // not last, write comma
			}

			ofile_digitized << "\n";

			is_first_event_with_digidata = true;
		}
	}

	// If we emitted a header, we have a stable column set and can write rows.
	if (is_first_event_with_digidata) {
		for (auto digi_hit : digitizedData) {
			// Argument passed to getter: 0 = do not get sro vars.
			const auto& imap = digi_hit->getIntObservablesMap(0);
			const auto& dmap = digi_hit->getDblObservablesMap(0);

			size_t total = dmap.size();
			size_t i     = 0;

			ofile_digitized << event_number << ", " << timestamp << ", " << thread_id << ", " << detectorName << ", ";

			for (const auto& [variableName, value] : imap) { ofile_digitized << value << ", "; }
			for (const auto& [variableName, value] : dmap) {
				ofile_digitized << value;
				if (++i < total) ofile_digitized << ", "; // not last, write comma
				else ofile_digitized << "\n";
			}
		}
	}

	return true;
}
