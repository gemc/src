// gstreamer
#include "gstreamerASCIIFactory.h"
#include "gstreamerConventions.h"

// Implementation summary:
// Write one detector true-information bank for the current event in text form.
// Use '\n' instead of std::endl so each line does not force a flush.

bool GstreamerTextFactory::publishEventTrueInfoDataImpl(const std::string&                       detectorName,
														const std::vector<const GTrueInfoData*>& trueInfoData) {
	if (!ofile.is_open()) { log->error(ERR_CANTOPENOUTPUT, SFUNCTION_NAME, "Error: can't access ", filename()); }

	ofile << GTAB << "Detector <" << detectorName << "> True Info Bank {\n";

	for (auto trueInfoHit : trueInfoData) {
		auto identifierString = getIdentityString(trueInfoHit->getIdentity());

		ofile << GTABTAB << "Hit address: " << identifierString << " {\n";

		for (const auto& [variableName, value] : trueInfoHit->getDoubleVariablesMap()) {
			ofile << GTABTABTAB << variableName << ": " << value << "\n";
		}
		for (const auto& [variableName, value] : trueInfoHit->getStringVariablesMap()) {
			ofile << GTABTABTAB << variableName << ": " << value << "\n";
		}

		ofile << GTABTAB << "}\n";
	}
	ofile << GTAB << "}\n";

	return true;
}

bool GstreamerTextFactory::publishEventGeneratedParticlesImpl(const std::string& bankName,
                                                              const GGeneratedParticleBank& particles) {
	if (!ofile.is_open()) { log->error(ERR_CANTOPENOUTPUT, SFUNCTION_NAME, "Error: can't access ", filename()); }

	ofile << GTAB << "Generated Particle Bank <" << bankName << "> {\n";

	for (const auto& particle : particles) {
		ofile << GTABTAB << "Particle {\n";
		ofile << GTABTABTAB << "name: " << particle.name << "\n";
		ofile << GTABTABTAB << "pid: " << particle.pid << "\n";
		ofile << GTABTABTAB << "type: " << particle.type << "\n";
		ofile << GTABTABTAB << "multiplicity: " << particle.multiplicity << "\n";
		ofile << GTABTABTAB << "p: " << particle.p << "\n";
		ofile << GTABTABTAB << "theta: " << particle.theta << "\n";
		ofile << GTABTABTAB << "phi: " << particle.phi << "\n";
		ofile << GTABTABTAB << "vx: " << particle.vx << "\n";
		ofile << GTABTABTAB << "vy: " << particle.vy << "\n";
		ofile << GTABTABTAB << "vz: " << particle.vz << "\n";
		ofile << GTABTAB << "}\n";
	}

	ofile << GTAB << "}\n";
	return true;
}
