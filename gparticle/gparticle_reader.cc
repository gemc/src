// gparticle
#include "gparticle_reader.h"
#include "gparticleConventions.h"
#include "gparticle_options.h"
#include "gparticle/readers/gparticle_lund_reader.h"

// gemc
#include "gfactory.h"
#include "gutilities.h"

// c++
#include <algorithm>

GParticleReader::GParticleReader(const std::shared_ptr<GOptions>& gopts) : GBase(gopts, GPARTICLE_LOGGER) {
}

GParticleEvents GParticleReader::loadParticleEvents(const GParticleSourceDefinition& source,
                                                    const std::shared_ptr<GLogger>& logger) {
	GParticleEvents events;
	auto            particles = loadParticles(source, logger);
	if (!particles.empty()) { events.emplace_back(std::move(particles)); }
	return events;
}

GParticleReader* GParticleReader::instantiate(dlhandle h, std::shared_ptr<GOptions> gopts) {
	if (!h) return nullptr;

	using fptr = GParticleReader* (*)(std::shared_ptr<GOptions>);

	auto sym = dlsym(h, "GParticleReaderFactory");
	if (!sym) return nullptr;

	auto func = reinterpret_cast<fptr>(sym);
	return func(gopts);
}

namespace gparticle {
const std::vector<std::string>& supported_static_reader_formats() {
	static const std::vector<std::string> formats = {"lund"};
	return formats;
}

std::vector<GParticleSourceDefinition> getGParticleSourceDefinitions(const std::shared_ptr<GOptions>& gopts) {
	std::vector<GParticleSourceDefinition> sources;

	if (!gopts->doesOptionExist("gparticlefile")) { return sources; }

	auto source_node = gopts->getOptionNode("gparticlefile");
	if (!source_node || source_node.IsNull() || !source_node.IsSequence()) { return sources; }

	for (auto source_item : source_node) {
		sources.emplace_back(
			gutilities::convertToLowercase(gopts->get_variable_in_option<std::string>(
				source_item, "format", goptions::NODFLT)),
			gopts->get_variable_in_option<std::string>(source_item, "filename", goptions::NODFLT)
		);
	}

	return sources;
}

std::vector<GparticlePtr> getGParticlesFromSources(const std::shared_ptr<GOptions>& gopts,
                                                   std::shared_ptr<GLogger>&        logger) {
	std::vector<GparticlePtr> particles;
	for (const auto& event : getGParticleEventsFromSources(gopts, logger)) {
		particles.insert(particles.end(), event.begin(), event.end());
	}
	return particles;
}

GParticleEvents getGParticleEventsFromSources(const std::shared_ptr<GOptions>& gopts,
                                              std::shared_ptr<GLogger>&        logger) {
	GParticleEvents           events;
	GManager                  manager(gopts);

	manager.RegisterObjectFactory<GParticleLundReader>("lund", gopts);

	for (const auto& source : getGParticleSourceDefinitions(gopts)) {
		std::shared_ptr<GParticleReader> reader;

		const auto& builtins = supported_static_reader_formats();
		if (std::find(builtins.begin(), builtins.end(), source.format) != builtins.end()) {
			reader = std::shared_ptr<GParticleReader>(manager.CreateObject<GParticleReader>(source.format));
		}
		else {
			reader = manager.LoadAndRegisterObjectFromLibrary<GParticleReader>(source.gparticlePluginName(), gopts);
		}

		if (reader == nullptr) {
			logger->error(ERR_GPARTICLEREADERNOTFOUND,
			              "Could not create gparticle reader for format <", source.format, ">");
			continue;
		}

		auto source_events = reader->loadParticleEvents(source, logger);
		if (source_events.size() > events.size()) { events.resize(source_events.size()); }

		for (size_t event_index = 0; event_index < source_events.size(); event_index++) {
			events[event_index].insert(events[event_index].end(),
			                           source_events[event_index].begin(),
			                           source_events[event_index].end());
		}
	}

	return events;
}
}
