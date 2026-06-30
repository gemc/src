// gstreamer
#include "gstreamerCSVFactory.h"
#include "gstreamerConventions.h"

// Implementation summary:
// Manage the lifetime of the two CSV streams used by the plugin.

bool GstreamerCsvFactory::openConnection() {
	// Both streams must be open for the CSV backend to operate correctly.
	if (ofile_true_info.is_open() && ofile_digitized.is_open() &&
	    ofile_generated.is_open() && ofile_generated_tracked.is_open()) {
		return true;
	}

	if (!ofile_true_info.is_open()) {
		ofile_true_info.clear();
		ofile_true_info.open(filename_true_info(), std::ios::out | std::ios::trunc);

		if (!ofile_true_info.is_open() || !ofile_true_info) {
			log->error(ERR_CANTOPENOUTPUT, SFUNCTION_NAME, " could not open file ", filename_true_info());
		}

		log->info(1, SFUNCTION_NAME, "GstreamerCsvFactory: opened file " + filename_true_info());
	}

	if (!ofile_digitized.is_open()) {
		ofile_digitized.clear();
		ofile_digitized.open(filename_digitized(), std::ios::out | std::ios::trunc);

		if (!ofile_digitized.is_open() || !ofile_digitized) {
			log->error(ERR_CANTOPENOUTPUT, SFUNCTION_NAME, " could not open file ", filename_digitized());
		}

		log->info(1, SFUNCTION_NAME, "GstreamerCsvFactory: opened file " + filename_digitized());
	}

	if (!ofile_generated.is_open()) {
		ofile_generated.clear();
		ofile_generated.open(filename_generated(), std::ios::out | std::ios::trunc);
		if (!ofile_generated.is_open() || !ofile_generated) {
			log->error(ERR_CANTOPENOUTPUT, SFUNCTION_NAME, " could not open file ", filename_generated());
		}
		ofile_generated << "evn, timestamp, thread_id, bank, name, pid, type, multiplicity, p, theta, phi, vx, vy, vz\n";
		log->info(1, SFUNCTION_NAME, "GstreamerCsvFactory: opened file " + filename_generated());
	}

	if (!ofile_generated_tracked.is_open()) {
		ofile_generated_tracked.clear();
		ofile_generated_tracked.open(filename_generated_tracked(), std::ios::out | std::ios::trunc);
		if (!ofile_generated_tracked.is_open() || !ofile_generated_tracked) {
			log->error(ERR_CANTOPENOUTPUT, SFUNCTION_NAME, " could not open file ", filename_generated_tracked());
		}
		ofile_generated_tracked << "evn, timestamp, thread_id, bank, name, pid, type, multiplicity, p, theta, phi, vx, vy, vz\n";
		log->info(1, SFUNCTION_NAME, "GstreamerCsvFactory: opened file " + filename_generated_tracked());
	}

	return true;
}

bool GstreamerCsvFactory::closeConnectionImpl() {
	// The public closeConnection() wrapper already flushes buffered events before this method runs.
	const bool had_ancestor_stream = ofile_ancestors.is_open();

	if (ofile_true_info.is_open()) ofile_true_info.close();
	if (ofile_digitized.is_open()) ofile_digitized.close();
	if (ofile_generated.is_open()) ofile_generated.close();
	if (ofile_generated_tracked.is_open()) ofile_generated_tracked.close();
	if (ofile_ancestors.is_open()) ofile_ancestors.close();

	if (ofile_true_info.is_open()) {
		log->error(ERR_CANTCLOSEOUTPUT, SFUNCTION_NAME, " could not close file " + filename_true_info());
	}
	if (ofile_digitized.is_open()) {
		log->error(ERR_CANTCLOSEOUTPUT, SFUNCTION_NAME, " could not close file " + filename_digitized());
	}
	if (ofile_generated.is_open()) {
		log->error(ERR_CANTCLOSEOUTPUT, SFUNCTION_NAME, " could not close file " + filename_generated());
	}
	if (ofile_generated_tracked.is_open()) {
		log->error(ERR_CANTCLOSEOUTPUT, SFUNCTION_NAME, " could not close file " + filename_generated_tracked());
	}
	if (ofile_ancestors.is_open()) {
		log->error(ERR_CANTCLOSEOUTPUT, SFUNCTION_NAME, " could not close file " + filename_ancestors());
	}

	log->info(1, SFUNCTION_NAME, "GstreamerCsvFactory: closed file " + filename_true_info());
	log->info(1, SFUNCTION_NAME, "GstreamerCsvFactory: closed file " + filename_digitized());
	log->info(1, SFUNCTION_NAME, "GstreamerCsvFactory: closed file " + filename_generated());
	log->info(1, SFUNCTION_NAME, "GstreamerCsvFactory: closed file " + filename_generated_tracked());
	if (had_ancestor_stream) {
		log->info(1, SFUNCTION_NAME, "GstreamerCsvFactory: closed file " + filename_ancestors());
	}

	return true;
}

std::ofstream& GstreamerCsvFactory::generated_stream_for_bank(const std::string& bankName) {
	return bankName == "generated_tracked" ? ofile_generated_tracked : ofile_generated;
}

bool GstreamerCsvFactory::publishEventGeneratedParticlesImpl(const std::string& bankName,
                                                             const GGeneratedParticleBank& particles) {
	auto& stream = generated_stream_for_bank(bankName);
	if (!stream.is_open()) {
		log->error(ERR_CANTOPENOUTPUT, SFUNCTION_NAME, "Error: can't access generated CSV stream");
	}

	for (const auto& particle : particles) {
		stream << event_number << ", " << timestamp << ", " << thread_id << ", " << bankName << ", "
		       << particle.name << ", "
		       << particle.pid << ", "
		       << particle.type << ", "
		       << particle.multiplicity << ", "
		       << particle.p << ", "
		       << particle.theta << ", "
		       << particle.phi << ", "
		       << particle.vx << ", "
		       << particle.vy << ", "
		       << particle.vz << "\n";
	}

	return true;
}

bool GstreamerCsvFactory::publishEventAncestorsImpl(const GAncestorBank& ancestors) {
	if (!ofile_ancestors.is_open()) {
		ofile_ancestors.open(filename_ancestors(), std::ios::out | std::ios::trunc);
		if (!ofile_ancestors.is_open() || !ofile_ancestors) {
			log->error(ERR_CANTOPENOUTPUT, SFUNCTION_NAME, " could not open file ", filename_ancestors());
		}
		ofile_ancestors << "evn, timestamp, thread_id, pid, tid, mtid, trackE, px, py, pz, vx, vy, vz\n";
	}

	for (const auto& ancestor : ancestors) {
		ofile_ancestors << event_number << ", " << timestamp << ", " << thread_id << ", "
		                << ancestor.pid << ", " << ancestor.tid << ", " << ancestor.mtid << ", "
		                << ancestor.trackE << ", " << ancestor.px << ", " << ancestor.py << ", "
		                << ancestor.pz << ", " << ancestor.vx << ", " << ancestor.vy << ", "
		                << ancestor.vz << "\n";
	}
	return true;
}
