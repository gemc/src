#pragma once

// gsystem
#include "systemFactory.h"

// sqlite
#include "sqlite3.h"


// system factory
class GSystemSQLiteFactory : GSystemFactory {

public:
	// inherit the base (const std::shared_ptr<GOptions>&) ctor
	using GSystemFactory::GSystemFactory;

private:
	void loadMaterials(GSystem* system) override;

	void loadGeometry(GSystem* system) override;

	void closeSystem() override;


	void initialize_sqlite_db(GSystem* system);

	sqlite3* db          = nullptr;
	std::string   system_name = "na";
	std::string   dbhost      = "na";
	std::string   variation   = "default";
	int      runno       = 0;


};
