// gstreamer
#include "gstreamerJSONFactory.h"
#include "gstreamerConventions.h"

// c++
#include <iomanip>

// Implementation summary:
// Manage the JSON output stream and the lifetime of the top-level JSON document.

bool GstreamerJsonFactory::openConnection() {
	if (ofile.is_open()) {
		// Already open for this streamer instance.
		return true;
	}

	ofile.clear();
	ofile.open(filename(), std::ios::out | std::ios::trunc);

	if (!ofile.is_open() || !ofile) {
		log->error(ERR_CANTOPENOUTPUT, SFUNCTION_NAME, " could not open file ", filename());
		return false;
	}

	log->info(1, SFUNCTION_NAME, "GstreamerJsonFactory: opened file " + filename());

	// The top-level JSON container depends on whether this instance ends up writing
	// events or frames, so defer initialization until the first publish call.
	is_file_initialized         = false;
	wrote_first_top_level_entry = false;
	top_level_type.clear();

	return true;
}

bool GstreamerJsonFactory::closeConnectionImpl() {
	// The public closeConnection() wrapper already flushes buffered events before this method runs.

	// Finalize the top-level JSON structure only if it was ever started.
	closeTopLevelObjectIfNeeded();

	if (ofile.is_open()) ofile.close();
	if (ofile.is_open()) {
		log->error(ERR_CANTCLOSEOUTPUT, SFUNCTION_NAME, " could not close file ", filename());
	}

	log->info(1, SFUNCTION_NAME, "GstreamerJsonFactory: closed file " + filename());

	return true;
}

void GstreamerJsonFactory::ensureFileInitializedForType(const std::string& type) {
	if (is_file_initialized) return;

	top_level_type = type;

	// Single top-level object with exactly one active array:
	//   { "type": "event",  "events": [ ... ] }
	// or
	//   { "type": "stream", "frames": [ ... ] }
	ofile << "{\n";
	ofile << "  \"type\": \"" << jsonEscape(type) << "\",\n";

	if (type == "event") {
		ofile << "  \"events\": [\n";
	}
	else {
		ofile << "  \"frames\": [\n";
	}

	is_file_initialized         = true;
	wrote_first_top_level_entry = false;
}

void GstreamerJsonFactory::writeTopLevelEntry(const std::string& entry_json) {
	// This helper assumes the correct top-level array has already been opened.
	if (!is_file_initialized) {
		log->error(ERR_PUBLISH_ERROR, "JSON file is not initialized in GstreamerJsonFactory::writeTopLevelEntry");
		return;
	}

	// Entries inside the top-level array are comma-separated.
	if (wrote_first_top_level_entry) {
		ofile << ",\n";
	}
	wrote_first_top_level_entry = true;

	// Keep a stable indentation level for readability.
	ofile << "    " << entry_json;
}

void GstreamerJsonFactory::closeTopLevelObjectIfNeeded() {
	if (!is_file_initialized) return;

	// Close the active top-level array and then the containing object.
	ofile << "\n  ]\n";
	ofile << "}\n";
	is_file_initialized = false;
}