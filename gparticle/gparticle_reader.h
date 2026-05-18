#pragma once

// gparticle
#include "gparticle.h"

// gemc
#include "gbase.h"
#include "gdl.h"
#include "goptions.h"

// c++
#include <memory>
#include <string>
#include <utility>
#include <vector>

using GParticleEvent  = std::vector<GparticlePtr>;
using GParticleEvents = std::vector<GParticleEvent>;

struct GParticleSourceDefinition
{
	GParticleSourceDefinition() = default;

	GParticleSourceDefinition(std::string f, std::string n)
		: format(std::move(f)), filename(std::move(n)) {
	}

	std::string format;
	std::string filename;

	[[nodiscard]] std::string gparticlePluginName() const { return "gparticle_" + format + "_plugin"; }
};

class GParticleReader : public GBase<GParticleReader>
{
public:
	explicit GParticleReader(const std::shared_ptr<GOptions>& gopts);
	~GParticleReader() override = default;

	virtual std::vector<GparticlePtr> loadParticles(const GParticleSourceDefinition& source,
	                                                const std::shared_ptr<GLogger>& logger) = 0;

	virtual GParticleEvents loadParticleEvents(const GParticleSourceDefinition& source,
	                                           const std::shared_ptr<GLogger>& logger);

	void set_loggers([[maybe_unused]] const std::shared_ptr<GOptions>& gopts) {
	}

	static GParticleReader* instantiate(dlhandle h, std::shared_ptr<GOptions> gopts);
};

namespace gparticle {
std::vector<GParticleSourceDefinition> getGParticleSourceDefinitions(const std::shared_ptr<GOptions>& gopts);

std::vector<GparticlePtr> getGParticlesFromSources(const std::shared_ptr<GOptions>& gopts,
                                                   std::shared_ptr<GLogger>&        logger);

GParticleEvents getGParticleEventsFromSources(const std::shared_ptr<GOptions>& gopts,
                                              std::shared_ptr<GLogger>&        logger);

const std::vector<std::string>& supported_static_reader_formats();
}
