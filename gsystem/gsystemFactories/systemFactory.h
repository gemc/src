#pragma once

// gemc
#include "goptions.h"
#include "glogger.h"
#include "gutilities.h"

// gsystem
#include "gsystem.h"


// system factory
class GSystemFactory {
public:
    // calls loadGeometry and loadMaterial
    // verbosity passed here comes from goptions gsystemv
    void loadSystem(GSystem *system, const std::shared_ptr<GLogger>& log) {

    	log->info(1, "Loading system <", system->getName(), "> using factory <", system->getFactoryName(), ">");

        possibleLocationOfFiles.emplace_back(".");

    	std::filesystem::path gemcRoot = gutilities::gemc_root();
    	possibleLocationOfFiles.push_back( gemcRoot.string());


        loadMaterials(system, log);
        loadGeometry(system, log);
    }

    virtual void closeSystem([[maybe_unused]] std::shared_ptr<GLogger>& log) {
        possibleLocationOfFiles.clear();
    }

    virtual ~GSystemFactory() = default;

private:
    virtual void loadMaterials(GSystem *system, std::shared_ptr<GLogger> log) = 0;

    virtual void loadGeometry(GSystem *system, std::shared_ptr<GLogger> log) = 0;

protected:
    vector <string> possibleLocationOfFiles;

public:
    void addPossibleFileLocation(const string& fl) {  possibleLocationOfFiles.push_back(fl); }

};
