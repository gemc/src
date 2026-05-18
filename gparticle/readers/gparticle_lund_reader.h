#pragma once

// gparticle
#include "gparticle/gparticle_reader.h"

class GParticleLundReader : public GParticleReader
{
public:
	using GParticleReader::GParticleReader;

	std::vector<GparticlePtr> loadParticles(const GParticleSourceDefinition& source,
	                                        const std::shared_ptr<GLogger>& logger) override;

	GParticleEvents loadParticleEvents(const GParticleSourceDefinition& source,
	                                   const std::shared_ptr<GLogger>& logger) override;
};
