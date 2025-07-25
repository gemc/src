#pragma once

// gsystem
#include "systemFactory.h"

// sqlite
#include "sqlite3.h"


// system factory
class GSystemSQLiteFactory : GSystemFactory {

public:
	// constructor will load the possible location(s) of the geometry and material databases
	GSystemSQLiteFactory();

private:
	void loadMaterials(GSystem* system, std::shared_ptr<GLogger> log) override;

	void loadGeometry(GSystem* system, std::shared_ptr<GLogger> log) override;

	void closeSystem(std::shared_ptr<GLogger>& log) override;


	void initialize_sqlite_db(GSystem* system, std::shared_ptr<GLogger>& log);

	sqlite3* db          = nullptr;
	std::string   system_name = "na";
	std::string   dbhost      = "na";
	std::string   variation   = "default";
	int      runno       = 0;


};
