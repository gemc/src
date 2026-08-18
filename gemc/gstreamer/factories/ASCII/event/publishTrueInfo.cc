// gstreamer
#include "gstreamerASCIIFactory.h"
#include "gstreamerConventions.h"

// Implementation summary:
// Write one detector true-information bank for the current event in text form.
// Use '\n' instead of std::endl so each line does not force a flush.

bool GstreamerTextFactory::publishEventTrueInfoDataImpl(const std::string&                       detectorName,
														const std::vector<const GTrueInfoData*>& trueInfoData) {
	if (!ofile.is_open()) {
		log->error(gstreamer::ERR_CANTOPENOUTPUT, SFUNCTION_NAME, "Error: can't access ", filename());
	}

	ofile << guts::GTAB << "Detector <" << detectorName << "> True Info Bank {\n";

	for (auto trueInfoHit : trueInfoData) {
		auto identifierString = getIdentityString(trueInfoHit->getIdentity());

		ofile << guts::GTABTAB << "Hit address: " << identifierString << " {\n";

		for (const auto& [variableName, value] : trueInfoHit->getDoubleVariablesMap()) {
			ofile << guts::GTABTABTAB << variableName << ": " << value << "\n";
		}
		for (const auto& [variableName, value] : trueInfoHit->getStringVariablesMap()) {
			ofile << guts::GTABTABTAB << variableName << ": " << value << "\n";
		}

		ofile << guts::GTABTAB << "}\n";
	}
	ofile << guts::GTAB << "}\n";

	return true;
}

bool GstreamerTextFactory::publishEventGeneratedParticlesImpl(const std::string& bankName,
                                                              const GGeneratedParticleBank& particles) {
	if (!ofile.is_open()) {
		log->error(gstreamer::ERR_CANTOPENOUTPUT, SFUNCTION_NAME, "Error: can't access ", filename());
	}

	ofile << guts::GTAB << "Generated Particle Bank <" << bankName << "> {\n";

	for (const auto& particle : particles) {
		ofile << guts::GTABTAB << "Particle {\n";
		ofile << guts::GTABTABTAB << "name: " << particle.name << "\n";
		ofile << guts::GTABTABTAB << "pid: " << particle.pid << "\n";
		ofile << guts::GTABTABTAB << "type: " << particle.type << "\n";
		ofile << guts::GTABTABTAB << "multiplicity: " << particle.multiplicity << "\n";
		ofile << guts::GTABTABTAB << "p: " << particle.p << "\n";
		ofile << guts::GTABTABTAB << "theta: " << particle.theta << "\n";
		ofile << guts::GTABTABTAB << "phi: " << particle.phi << "\n";
		ofile << guts::GTABTABTAB << "vx: " << particle.vx << "\n";
		ofile << guts::GTABTABTAB << "vy: " << particle.vy << "\n";
		ofile << guts::GTABTABTAB << "vz: " << particle.vz << "\n";
		ofile << guts::GTABTAB << "}\n";
	}

	ofile << guts::GTAB << "}\n";
	return true;
}

bool GstreamerTextFactory::publishEventAncestorsImpl(const GAncestorBank& ancestors) {
	if (!ofile.is_open()) {
		log->error(gstreamer::ERR_CANTOPENOUTPUT, SFUNCTION_NAME, "Error: can't access ", filename());
	}

	ofile << guts::GTAB << "Ancestor Bank {\n";
	for (const auto& ancestor : ancestors) {
		ofile << guts::GTABTAB << "Track {\n";
		ofile << guts::GTABTABTAB << "pid: " << ancestor.pid << "\n";
		ofile << guts::GTABTABTAB << "tid: " << ancestor.tid << "\n";
		ofile << guts::GTABTABTAB << "mtid: " << ancestor.mtid << "\n";
		ofile << guts::GTABTABTAB << "trackE: " << ancestor.trackE << "\n";
		ofile << guts::GTABTABTAB << "px: " << ancestor.px << "\n";
		ofile << guts::GTABTABTAB << "py: " << ancestor.py << "\n";
		ofile << guts::GTABTABTAB << "pz: " << ancestor.pz << "\n";
		ofile << guts::GTABTABTAB << "vx: " << ancestor.vx << "\n";
		ofile << guts::GTABTABTAB << "vy: " << ancestor.vy << "\n";
		ofile << guts::GTABTABTAB << "vz: " << ancestor.vz << "\n";
		ofile << guts::GTABTAB << "}\n";
	}
	ofile << guts::GTAB << "}\n";
	return true;
}
