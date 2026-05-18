// gparticle
#include "gparticle/readers/gparticle_lund_reader.h"
#include "gparticle/gparticleConventions.h"

// geant4
#include "G4ParticleTable.hh"

// c++
#include <cmath>
#include <fstream>
#include <sstream>
#include <utility>
#include <vector>

namespace {
constexpr size_t LUND_MIN_HEADER_COLUMNS = 10;
constexpr size_t LUND_MAX_HEADER_COLUMNS = 100;
constexpr size_t LUND_MIN_PARTICLE_COLUMNS = 14;
constexpr size_t LUND_MAX_PARTICLE_COLUMNS = 100;
constexpr int    LUND_PROPAGATED_TYPE    = 1;

bool is_blank_line(const std::string& line) {
	return line.find_first_not_of(" \t\r\n") == std::string::npos;
}

std::vector<double> parse_lund_header(const std::string& line) {
	std::istringstream stream(line);
	std::vector<double> values;
	double value = 0;

	while (stream >> value) { values.emplace_back(value); }
	stream >> std::ws;
	if (!stream.eof()) { values.clear(); }

	return values;
}

struct LundParticleLine
{
	int    index = 0;
	double lifetime = 0;
	int    type = 0;
	int    pid = 0;
	int    parent = 0;
	int    daughter = 0;
	double px = 0;
	double py = 0;
	double pz = 0;
	double energy = 0;
	double mass = 0;
	double vx = 0;
	double vy = 0;
	double vz = 0;
};

bool parse_lund_particle_line(const std::string& line, LundParticleLine& particle) {
	std::istringstream stream(line);
	std::vector<double> values;
	double value = 0;

	while (stream >> value) { values.emplace_back(value); }
	stream >> std::ws;
	if (!stream.eof() ||
	    values.size() < LUND_MIN_PARTICLE_COLUMNS ||
	    values.size() > LUND_MAX_PARTICLE_COLUMNS) {
		return false;
	}

	particle.index    = static_cast<int>(values[0]);
	particle.lifetime = values[1];
	particle.type     = static_cast<int>(values[2]);
	particle.pid      = static_cast<int>(values[3]);
	particle.parent   = static_cast<int>(values[4]);
	particle.daughter = static_cast<int>(values[5]);
	particle.px       = values[6];
	particle.py       = values[7];
	particle.pz       = values[8];
	particle.energy   = values[9];
	particle.mass     = values[10];
	particle.vx       = values[11];
	particle.vy       = values[12];
	particle.vz       = values[13];

	return particle.index == values[0] &&
	       particle.type == values[2] &&
	       particle.pid == values[3] &&
	       particle.parent == values[4] &&
	       particle.daughter == values[5];
}

std::string particle_name_from_pid(int pid, const std::shared_ptr<GLogger>& logger) {
	auto particle_table = G4ParticleTable::GetParticleTable();
	if (particle_table == nullptr) {
		logger->error(ERR_GPARTICLETABLENOTFOUND, "G4ParticleTable not found while reading Lund particles");
		return {};
	}

	auto particle_definition = particle_table->FindParticle(pid);
	if (particle_definition == nullptr) {
		logger->error(ERR_GPARTICLENOTFOUND, "Lund pid <", pid, "> was not found in G4ParticleTable");
		return {};
	}

	return particle_definition->GetParticleName();
}

GparticlePtr make_gparticle_from_lund(const LundParticleLine& particle, const std::shared_ptr<GLogger>& logger) {
	const auto particle_name = particle_name_from_pid(particle.pid, logger);
	if (particle_name.empty()) { return nullptr; }

	const double momentum = std::sqrt(
		particle.px * particle.px +
		particle.py * particle.py +
		particle.pz * particle.pz
	);

	double theta = 0;
	if (momentum > 0) { theta = std::acos(particle.pz / momentum); }

	const double phi = std::atan2(particle.py, particle.px);

	return std::make_shared<Gparticle>(
		particle_name,
		1,
		momentum,
		0,
		"GeV",
		"uniform",
		theta,
		0,
		"uniform",
		phi,
		0,
		"rad",
		particle.vx,
		particle.vy,
		particle.vz,
		0,
		0,
		0,
		"cm",
		"uniform",
		logger,
		particle.type
	);
}

GParticleRecord make_record_from_lund(const LundParticleLine& particle,
                                      const std::shared_ptr<GLogger>& logger) {
	(void)logger;

	const double momentum = std::sqrt(
		particle.px * particle.px +
		particle.py * particle.py +
		particle.pz * particle.pz
	);

	double theta = 0;
	if (momentum > 0) { theta = std::acos(particle.pz / momentum); }

	std::string particle_name;
	auto particle_table = G4ParticleTable::GetParticleTable();
	if (particle_table != nullptr) {
		auto particle_definition = particle_table->FindParticle(particle.pid);
		if (particle_definition != nullptr) { particle_name = particle_definition->GetParticleName(); }
	}
	if (particle_name.empty()) { particle_name = std::to_string(particle.pid); }

	return {
		particle_name,
		particle.pid,
		particle.type,
		1,
		momentum,
		theta,
		std::atan2(particle.py, particle.px),
		particle.vx,
		particle.vy,
		particle.vz
	};
}

}

GParticleEvents GParticleLundReader::loadParticleEvents(const GParticleSourceDefinition& source,
                                                        const std::shared_ptr<GLogger>& logger,
                                                        bool propagated_only) {
	GParticleEvents           events;
	std::ifstream             input(source.filename);

	if (!input.is_open()) {
		logger->error(ERR_GPARTICLEFILEOPEN, "Could not open Lund particle file <", source.filename, ">");
		return events;
	}

	std::string line;
	while (std::getline(input, line)) {
		if (is_blank_line(line)) { continue; }

		const auto header_values = parse_lund_header(line);

		if (header_values.size() < LUND_MIN_HEADER_COLUMNS || header_values.size() > LUND_MAX_HEADER_COLUMNS) {
			logger->error(ERR_GPARTICLEFILEFORMAT, "Malformed Lund event header in <", source.filename, ">: ", line);
			continue;
		}

		const auto particle_count = static_cast<int>(header_values.front());
		if (particle_count < 0 || particle_count != header_values.front()) {
			logger->error(ERR_GPARTICLEFILEFORMAT,
			              "Lund event header first column must be a non-negative integer in <",
			              source.filename, ">: ", line);
			continue;
		}

		bool have_first_particle_line = false;
		if (!std::getline(input, line)) {
			logger->error(ERR_GPARTICLEFILEFORMAT,
			              "Lund event header must be followed by a blank line in <", source.filename, ">");
			continue;
		}
		if (!is_blank_line(line)) { have_first_particle_line = true; }

		GParticleEvent event_particles;
		for (int i = 0; i < particle_count; i++) {
			if (have_first_particle_line) {
				have_first_particle_line = false;
			}
			else if (!std::getline(input, line)) {
				logger->error(ERR_GPARTICLEFILEFORMAT,
				              "Lund event declared ", particle_count, " particles but ended after ",
				              i, " particle lines in <", source.filename, ">");
				break;
			}

			if (is_blank_line(line)) {
				logger->error(ERR_GPARTICLEFILEFORMAT,
				              "Unexpected blank line inside Lund particle block in <", source.filename, ">");
				continue;
			}

			LundParticleLine lund_particle;
			if (!parse_lund_particle_line(line, lund_particle)) {
				logger->error(ERR_GPARTICLEFILEFORMAT, "Malformed Lund particle line in <", source.filename, ">: ", line);
				continue;
			}

			if (lund_particle.index != i + 1) {
				logger->error(ERR_GPARTICLEFILEFORMAT,
				              "Lund particle index must start from 1 and follow particle order in <",
				              source.filename, ">: ", line);
				continue;
			}

			if (propagated_only && lund_particle.type != LUND_PROPAGATED_TYPE) { continue; }

			auto particle = make_gparticle_from_lund(lund_particle, logger);
			if (particle != nullptr) { event_particles.emplace_back(particle); }
		}

		events.emplace_back(std::move(event_particles));
	}

	logger->info(1, "Loaded ", events.size(), " Lund events from <", source.filename, ">");
	return events;
}

std::vector<GparticlePtr> GParticleLundReader::loadParticles(const GParticleSourceDefinition& source,
                                                            const std::shared_ptr<GLogger>& logger) {
	std::vector<GparticlePtr> particles;
	for (const auto& event : loadParticleEvents(source, logger)) {
		particles.insert(particles.end(), event.begin(), event.end());
	}

	logger->info(1, "Loaded ", particles.size(), " propagated particles from Lund file <", source.filename, ">");
	return particles;
}

GParticleRecordEvents GParticleLundReader::loadParticleRecordEvents(const GParticleSourceDefinition& source,
                                                                    const std::shared_ptr<GLogger>& logger) {
	GParticleRecordEvents events;
	std::ifstream         input(source.filename);

	if (!input.is_open()) {
		logger->error(ERR_GPARTICLEFILEOPEN, "Could not open Lund particle file <", source.filename, ">");
		return events;
	}

	std::string line;
	while (std::getline(input, line)) {
		if (is_blank_line(line)) { continue; }

		const auto header_values = parse_lund_header(line);
		if (header_values.size() < LUND_MIN_HEADER_COLUMNS || header_values.size() > LUND_MAX_HEADER_COLUMNS) {
			logger->error(ERR_GPARTICLEFILEFORMAT, "Malformed Lund event header in <", source.filename, ">: ", line);
			continue;
		}

		const auto particle_count = static_cast<int>(header_values.front());
		if (particle_count < 0 || particle_count != header_values.front()) {
			logger->error(ERR_GPARTICLEFILEFORMAT,
			              "Lund event header first column must be a non-negative integer in <",
			              source.filename, ">: ", line);
			continue;
		}

		bool have_first_particle_line = false;
		if (!std::getline(input, line)) {
			logger->error(ERR_GPARTICLEFILEFORMAT,
			              "Lund event header must be followed by a blank line in <", source.filename, ">");
			continue;
		}
		if (!is_blank_line(line)) { have_first_particle_line = true; }

		GParticleRecordEvent event_particles;
		for (int i = 0; i < particle_count; i++) {
			if (have_first_particle_line) {
				have_first_particle_line = false;
			}
			else if (!std::getline(input, line)) {
				logger->error(ERR_GPARTICLEFILEFORMAT,
				              "Lund event declared ", particle_count, " particles but ended after ",
				              i, " particle lines in <", source.filename, ">");
				break;
			}

			if (is_blank_line(line)) {
				logger->error(ERR_GPARTICLEFILEFORMAT,
				              "Unexpected blank line inside Lund particle block in <", source.filename, ">");
				continue;
			}

			LundParticleLine lund_particle;
			if (!parse_lund_particle_line(line, lund_particle)) {
				logger->error(ERR_GPARTICLEFILEFORMAT, "Malformed Lund particle line in <", source.filename, ">: ", line);
				continue;
			}

			if (lund_particle.index != i + 1) {
				logger->error(ERR_GPARTICLEFILEFORMAT,
				              "Lund particle index must start from 1 and follow particle order in <",
				              source.filename, ">: ", line);
				continue;
			}

			event_particles.emplace_back(make_record_from_lund(lund_particle, logger));
		}

		events.emplace_back(std::move(event_particles));
	}

	logger->info(1, "Loaded ", events.size(), " Lund generated-particle record events from <", source.filename, ">");
	return events;
}
