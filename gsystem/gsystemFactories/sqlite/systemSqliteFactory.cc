// string for gexit
#include "gutilities.h"

// gsystem
#include "systemSqliteFactory.h"
#include "gsystemConventions.h"


GSystemSQLiteFactory::GSystemSQLiteFactory() = default;

void GSystemSQLiteFactory::initialize_sqlite_db(GSystem* system, std::shared_ptr<GLogger>& log) {
	// Save parameters from the system.
	system_name = system->getName();
	variation   = system->getVariation();
	runno       = system->getRunno();

	// Use system dbhost if not already set.
	if (dbhost == "na") {
		dbhost = system->get_dbhost();
	}
	log->info(1, "GSystemSQLiteFactory: dbhost set to <", dbhost, ">");

	// Attempt to open the primary database file.
	int rc = sqlite3_open_v2(dbhost.c_str(), &db, SQLITE_OPEN_READWRITE, nullptr);
	if (rc != SQLITE_OK) {
		// If opening failed, close the handle (if non-null) and set it to nullptr.
		if (db) {
			sqlite3_close(db);
			db = nullptr;
		}
		// Try alternative locations from possibleLocationOfFiles.
		for (const auto& trialLocation : possibleLocationOfFiles) {
			string newName = trialLocation + "/" + dbhost;
			log->info(1, "Trying sqlite file ", newName);

			rc = sqlite3_open_v2(newName.c_str(), &db, SQLITE_OPEN_READWRITE, nullptr);
			if (rc == SQLITE_OK) {
				log->info(1, "Sqlite file >" + newName + "< opened successfully");
				return;
			}
			else {
				// Close and reset db if open failed.
				if (db) {
					sqlite3_close(db);
					db = nullptr;
				}
			}
		}
	}
	else {
		log->info(1, "Sqlite file >" + dbhost + "< opened successfully");
		return;
	}

	// If we are here, no valid database file was found.
	log->error(ERR_GSETUPFILENOTOFOUND, "Sqlite database >" + dbhost + "< not found");
}


void GSystemSQLiteFactory::closeSystem( std::shared_ptr<GLogger>& log) {
	if (db) {
		sqlite3_close(db);
		db = nullptr; // Reset the pointer after closing
	}
	possibleLocationOfFiles.clear();
	log->info(1, "Closing sqlite database >", dbhost, "<");
}
