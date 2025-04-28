#ifndef SYSTEMFACTORY_H
#define SYSTEMFACTORY_H 1

// gemc
#include "goptions.h"
#include "glogger.h"
#include "gutilities.h"

// gsystem
#include "../gsystem.h"
#include "../gsystemConventions.h"


// system factory
class GSystemFactory {
public:
    // calls loadGeometry and loadMaterial
    // verbosity passed here comes from goptions gsystemv
    void loadSystem(GSystem *system, const std::shared_ptr<GLogger>& log) {

    	log->info(1, "Loading system ", system->getName(), " using factory ", system->getFactoryName());

        possibleLocationOfFiles.emplace_back(".");

        // environment for cad
        auto gtextEnv = getenv(GEMCDB_ENV); // char*
        if (gtextEnv != nullptr) {
            vector <string> dirsDB = gutilities::getStringVectorFromString(gtextEnv);

            if (!dirsDB.empty()) {
                for (auto &dirDB: dirsDB) {
                    possibleLocationOfFiles.push_back(dirDB);
                }
            }
        }

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

#endif
