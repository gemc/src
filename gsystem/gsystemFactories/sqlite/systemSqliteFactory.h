#ifndef SYSTEMSQLITEFACTORY_H
#define SYSTEMSQLITEFACTORY_H 1

// gsystem
#include "systemFactory.h"

// sqlite
#include "sqlite3.h"


// system factory
class GSystemSQLiteFactory : GSystemFactory
{

public:

	// constructor will load the possible location(s) of the geometry and material databases
    GSystemSQLiteFactory();

private:

	virtual void loadMaterials(GSystem *system, int verbosity);
	virtual void loadGeometry(GSystem *system, int verbosity);
    virtual void closeSystem();


    void initialize_sqlite_db(GSystem *system, int verbosity);

    sqlite3 *db = nullptr;
    string system_name = "na";
    string sqlite_file = "na";
    string variation = "default";
    int runno = 0;


};

#endif
