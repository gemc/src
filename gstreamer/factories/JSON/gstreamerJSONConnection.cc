// gstreamer
#include "gstreamerJSONFactory.h"
#include "gstreamerConventions.h"

// c++
#include <iomanip>

bool GstreamerJsonFactory::openConnection() {
	if (ofile.is_open()) {
		// Already open for this thread; nothing to do.
		return true;
	}

	ofile.clear();
	ofile.open(filename(), std::ios::out | std::ios::trunc);

	if (!ofile.is_open() || !ofile) {
		log->error(ERR_CANTOPENOUTPUT, SFUNCTION_NAME, " could not open file ", filename());
		return false;
	}

	log->info(0, "GstreamerJsonFactory: opened file " + filename());

	// Defer writing the top-level JSON object until we know whether this is an event or stream writer.
	is_file_initialized         = false;
	wrote_first_top_level_entry = false;
	top_level_type.clear();

	return true;
}

bool GstreamerJsonFactory::closeConnectionImpl() {
	// Ensure buffered events are written before closing the JSON structure.
	flushEventBuffer();

	// Close the JSON document if we ever started it.
	closeTopLevelObjectIfNeeded();

	if (ofile.is_open()) ofile.close();
	if (ofile.is_open()) {
		log->error(ERR_CANTCLOSEOUTPUT, SFUNCTION_NAME, " could not close file ", filename());
		return false;
	}

	return true;
}

void GstreamerJsonFactory::ensureFileInitializedForType(const std::string& type) {
	if (is_file_initialized) return;

	top_level_type = type;

	// Single top-level object with a single array.
	// { "type":"event", "events":[ ... ] }
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
	// The caller must have initialized the file for the correct type.
	if (!is_file_initialized) {
		log->error(ERR_PUBLISH_ERROR, "JSON file is not initialized in GstreamerJsonFactory::writeTopLevelEntry");
		return;
	}

	// Comma-separate entries inside the array.
	if (wrote_first_top_level_entry) {
		ofile << ",\n";
	}
	wrote_first_top_level_entry = true;

	// Indent entries consistently.
	ofile << "    " << entry_json;
}

void GstreamerJsonFactory::closeTopLevelObjectIfNeeded() {
	if (!is_file_initialized) return;

	// Close array and object.
	ofile << "\n  ]\n";
	ofile << "}\n";
	is_file_initialized = false;
}
