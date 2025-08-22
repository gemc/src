#pragma once

// gemc
#include "goptions.h"
#include "gbase.h"
#include "gutilities.h"

// gsystem
#include "gsystem_options.h"


// system factory
class GSystemFactory: public GBase<GSystemFactory> {
public:
	~GSystemFactory() override = default;

	explicit GSystemFactory(const std::shared_ptr<GOptions>& g) : GBase(g, GSFACTORY_LOGGER) {}

	// calls loadGeometry and loadMaterial
	// verbosity passed here comes from goptions gsystemv
	void loadSystem(GSystem* system) {

		log->info(1, "Loading system <", system->getName(), "> using factory <", system->getFactoryName(), ">");

		possibleLocationOfFiles.emplace_back(".");

		std::filesystem::path gemcRoot = gutilities::gemc_root();
		possibleLocationOfFiles.push_back(gemcRoot.string());
		possibleLocationOfFiles.push_back(system->get_dbhost());

		loadMaterials(system);
		loadGeometry(system);
	}

	virtual void closeSystem() { possibleLocationOfFiles.clear(); }


private:
	virtual void loadMaterials(GSystem* system) = 0;

	virtual void loadGeometry(GSystem* system) = 0;

protected:
	std::vector<std::string> possibleLocationOfFiles;

public:
	void addPossibleFileLocation(const std::string& fl) { possibleLocationOfFiles.push_back(fl); }

};
