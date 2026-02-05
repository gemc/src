#pragma once

// gemc
#include "goptions.h"
#include "gbase.h"
#include "gutilities.h"

// gsystem
#include "gsystem_options.h"

/**
 * \ingroup gemc_gsystem_factories
 *
 * \class GSystemFactory
 * \brief Abstract base class for loading a GSystem from a specific source.
 *
 * Concrete factories implement:
 * - \ref GSystemFactory::loadMaterials "loadMaterials()" to populate system materials.
 * - \ref GSystemFactory::loadGeometry  "loadGeometry()"  to populate system volumes.
 *
 * The public orchestration method \ref GSystemFactory::loadSystem "loadSystem()":
 * - logs the start of loading;
 * - seeds \c possibleLocationOfFiles with default search locations;
 * - calls \c loadMaterials() and then \c loadGeometry().
 *
 * Search-path behavior:
 * - "." is always added first.
 * - GEMC installation root (from gutilities) is added.
 * - system->get_dbhost() is also added, which can act as either a DB name or a directory hint
 *   depending on the concrete factory.
 *
 * \note Factories generally keep temporary state during load; they should release resources
 * in \ref GSystemFactory::closeSystem "closeSystem()".
 */
class GSystemFactory : public GBase<GSystemFactory>
{
public:
	~GSystemFactory() override = default;

	/**
	 * \brief Construct the factory with shared configuration.
	 *
	 * \param g Shared options/configuration instance.
	 */
	explicit GSystemFactory(const std::shared_ptr<GOptions>& g) : GBase(g, GSFACTORY_LOGGER) {
	}

	/**
	 * \brief Load a system by invoking the concrete factory steps.
	 *
	 * \param system Target system instance to populate.
	 *
	 * \details Sequence:
	 * - initialize default search locations;
	 * - load materials first (so geometry can reference material names);
	 * - load geometry (volumes) second.
	 */
	void loadSystem(GSystem* system) {
		log->info(1, "Loading system <", system->getName(), "> using factory <", system->getFactoryName(), ">");

		possibleLocationOfFiles.emplace_back(".");

		std::filesystem::path gemcRoot = gutilities::gemc_root();
		possibleLocationOfFiles.push_back(gemcRoot.string());
		possibleLocationOfFiles.push_back(system->get_dbhost());

		loadMaterials(system);
		loadGeometry(system);
	}

	/**
	 * \brief Factory cleanup hook.
	 *
	 * Concrete factories override this when they own external resources
	 * (e.g. open sqlite handles, file streams, cached state).
	 */
	virtual void closeSystem() { possibleLocationOfFiles.clear(); }

private:
	/**
	 * \brief Load materials into \c system.
	 *
	 * \param system Target system instance to populate.
	 */
	virtual void loadMaterials(GSystem* system) = 0;

	/**
	 * \brief Load geometry volumes into \c system.
	 *
	 * \param system Target system instance to populate.
	 */
	virtual void loadGeometry(GSystem* system) = 0;

protected:
	/// List of candidate directories used by file-based factories.
	std::vector<std::string> possibleLocationOfFiles;

public:
	/**
	 * \brief Add a candidate directory for file searches.
	 *
	 * \param fl Directory path to append.
	 */
	void addPossibleFileLocation(const std::string& fl) { possibleLocationOfFiles.push_back(fl); }
};
