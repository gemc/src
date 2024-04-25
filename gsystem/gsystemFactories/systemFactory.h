#ifndef SYSTEMFACTORY_H
#define SYSTEMFACTORY_H 1

// glibrary
#include "goptions.h"
#include "gutsConventions.h"

// gsystem
#include "../gsystem.h"

// c++
#include <fstream>
#include <iostream>
using std::ifstream;
using std::cout;
using std::endl;

// system factory
class GSystemFactory
{
public:
	// calls loadGeometry and loadMaterial
	// verbosity passed here comes from goptions gsystemv
	// a local variable verbosity is not used (if we did we'd need a dedicated constructor)
	void loadSystem(GSystem *system, int verbosity) {

		if(verbosity >= GVERBOSITY_SUMMARY) {
			cout << GSYSTEMLOGHEADER << "Loading system <" << KWHT << system->getName() << RST << "> using factory <" << system->getFactoryName() << ">" <<  endl;
		}

		possibleLocationOfFiles.push_back(".");
		possibleLocationOfFiles.push_back("");

		// environment for cad
		auto gtextEnv = getenv(GEMCDB_ENV); // char*
		if ( gtextEnv != nullptr) {
			vector<string> dirsDB = getStringVectorFromString(gtextEnv);

			if ( dirsDB.size() > 0) {
				for ( auto& dirDB: dirsDB ) {
					possibleLocationOfFiles.push_back(dirDB);
				}
			}
		}

		loadMaterials(system, verbosity);
		loadGeometry(system, verbosity);
	}

    virtual void closeSystem() {
        possibleLocationOfFiles.clear();
    }

	virtual ~GSystemFactory() = default;

private:
	virtual void loadMaterials(GSystem *system, int verbosity) = 0;
	virtual void loadGeometry(GSystem *system, int verbosity) = 0;

protected:
	vector<string> possibleLocationOfFiles;

public:
	void addPossibleFileLocation(string fl) { possibleLocationOfFiles.push_back(fl);}

};

#endif
