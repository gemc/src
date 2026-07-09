#pragma once

// gemc
#include <gemc/goptions/goptions.h>
#include <gemc/gbase/gbase.h>
#include <gemc/guts/gutilities.h>

// gsystem
#include <gemc/gsystem/gsystem_options.h>

/**
 * \ingroup gemc_gsystem_factories
 *
 * \class GSystemFactory
 * \brief Abstract base class for loading a GSystem from a specific source.
 *
 * Concrete factories implement two core steps:
 * - material loading (so volumes can reference material names);
 * - geometry loading (volumes and placement metadata).
 *
 * The public orchestration method \ref GSystemFactory::loadSystem "loadSystem()":
 * - logs the start of loading;
 * - seeds \c possibleLocationOfFiles with default search locations;
 * - invokes the concrete material-loading step;
 * - invokes the concrete geometry-loading step.
 *
 * Search-path behavior:
 * - \c "." is always added first.
 * - GEMC installation root (from gutilities) is added.
 * - \c system->get_dbhost() is also added, which can act as either a DB name or a directory hint
 *   depending on the concrete factory.
 *
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
	 *
	 * The base class does not enforce additional invariants beyond calling order.
	 * Concrete factories are responsible for:
	 * - validating input files / database connectivity;
	 * - logging and erroring out on unrecoverable conditions;
	 * - leaving the system in a consistent state on success.
	 */
	void loadSystem(GSystem* system) {
		log->info(1, "Loading system <", system->getName(), "> using factory <", system->getFactoryName(), ">");

		// Default: allow local execution context to resolve relative paths.
		possibleLocationOfFiles.emplace_back(".");

		// GEMC installation root is a common fallback location for resources.
		std::filesystem::path gemcRoot = gutilities::gemc_root();
		possibleLocationOfFiles.push_back(gemcRoot.string());

		// Some factories interpret dbhost as a directory hint; others interpret it as a filename.
		possibleLocationOfFiles.push_back(system->get_dbhost());

		// Materials first, then mirrors (which may reference materials), then geometry.
		loadMaterials(system);
		loadMirrors(system);
		loadGeometry(system);
	}

	/**
	 * \brief Factory cleanup hook.
	 *
	 * Concrete factories override this when they own external resources
	 * (e.g. open sqlite handles, file streams, cached state).
	 *
	 * The base implementation clears \c possibleLocationOfFiles.
	 */
	virtual void closeSystem() { possibleLocationOfFiles.clear(); }

private:
	/**
	 * \brief Load materials into \c system.
	 *
	 * \param system Target system instance to populate.
	 *
	 * \details
	 * Concrete factories implement this to create and insert GMaterial objects into the system.
	 * Implementations should treat the materials file/table as optional or mandatory according
	 * to the factory design (e.g. ASCII materials may be optional; sqlite materials may be empty).
	 */
	virtual void loadMaterials(GSystem* system) = 0;

	/**
	 * \brief Load mirrors (optical surface definitions) into \c system.
	 *
	 * \param system Target system instance to populate.
	 *
	 * \details
	 * Mirrors are optional: the default implementation is a no-op so factories without
	 * a mirror source (CAD, GDML) need not override it. Database/ASCII factories
	 * override this to create and insert GMirror objects into the system.
	 */
	virtual void loadMirrors([[maybe_unused]] GSystem* system) {
	}

	/**
	 * \brief Load geometry volumes into \c system.
	 *
	 * \param system Target system instance to populate.
	 *
	 * \details
	 * Concrete factories implement this to create and insert GVolume objects into the system.
	 * Geometry is typically mandatory for a fully-defined system unless annotations specify
	 * a materials-only workflow.
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
	 *
	 * \details Factories may accumulate locations from multiple sources (YAML directories,
	 * install roots, user-specified paths). Locations are searched in insertion order.
	 */
	void addPossibleFileLocation(const std::string& fl) { possibleLocationOfFiles.push_back(fl); }
};
