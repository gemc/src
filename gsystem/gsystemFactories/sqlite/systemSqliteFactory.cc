// string for gexit
#include "gutilities.h"

// gsystem
#include "systemSqliteFactory.h"
#include "gsystemConventions.h"

bool  GSystemSQLiteFactory::table_exists(sqlite3* db, const char* name) {
	sqlite3_stmt* st = nullptr;
	const char* sql = "SELECT 1 FROM sqlite_master WHERE type='table' AND name=? LIMIT 1";
	if (sqlite3_prepare_v2(db, sql, -1, &st, nullptr) != SQLITE_OK) return false;
	sqlite3_bind_text(st, 1, name, -1, SQLITE_TRANSIENT);
	bool exists = (sqlite3_step(st) == SQLITE_ROW);
	sqlite3_finalize(st);
	return exists;
}


void GSystemSQLiteFactory::initialize_sqlite_db(GSystem* system) {
	// skip ROOT system
	if (system->getName() == ROOTWORLDGVOLUMENAME) { return; }

	// Save parameters from the system.
	system_name = system->getName();
	variation   = system->getVariation();
	runno       = system->getRunno();

	// Use system dbhost if not already set.
	if (dbhost == "na") { dbhost = system->get_dbhost(); }
	log->info(1, "GSystemSQLiteFactory: dbhost set to <", dbhost, ">");

	// try local dir, gemc installation, then example dir
	std::vector<std::string> dirs = {
		".",
		gutilities::gemc_root().string(),
		(gutilities::gemc_root() / "examples").string()
	};

	auto dbPath = gutilities::searchForFileInLocations(dirs, dbhost);
	if (!dbPath) { log->error(ERR_GSQLITEERROR, "Failed to find database file. Exiting."); }

	if (sqlite3_open_v2(dbPath.value().c_str(), &db, SQLITE_OPEN_READONLY, nullptr) != SQLITE_OK) {
		sqlite3_close(db);
		db = nullptr;
		log->error(ERR_GSQLITEERROR, " Failed to open or validate database", dbhost);
	}
	log->info(1, "Opened database: " + dbhost, " found at ", dbPath.value());
	return;

	// If we are here, no valid database file was found.
	log->error(ERR_GSETUPFILENOTOFOUND, "Sqlite database >" + dbhost + "< not found");
}


void GSystemSQLiteFactory::closeSystem() {
	if (db) {
		sqlite3_close(db);
		db = nullptr; // Reset the pointer after closing
		log->info(1, "Closing sqlite database >", dbhost, "<");
	}
	possibleLocationOfFiles.clear();
}
