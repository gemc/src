// gsystem
#include "systemSqliteFactory.h"
#include "gsystemConventions.h"

void GSystemSQLiteFactory::loadGeometry(GSystem* system, std::shared_ptr<GLogger> log) {
	// Initialize the DB if needed.
	if (db == nullptr) { initialize_sqlite_db(system, log); }

	// Check that db is valid.
	if (db == nullptr) { log->error(ERR_GSQLITEERROR, "Database pointer is still null after initialization."); }


	const char* sql_query =
		"SELECT DISTINCT * FROM geometry WHERE experiment = ? AND system = ? AND variation = ? AND run = ?";
	sqlite3_stmt* stmt = nullptr;
	int           rc   = sqlite3_prepare_v2(db, sql_query, -1, &stmt, nullptr);
	if (rc != SQLITE_OK) {
		log->error(ERR_GSQLITEERROR, "Sqlite error preparing count query in loadGeometry: ",
		           sqlite3_errmsg(db), " (", rc, ") using query: ", sql_query);
	}

	// Bind parameters.
	string experiment  = system->getExperiment();
	string system_name = system->getName();
	string variation   = system->getVariation();
	int    runno       = system->getRunno();

	rc = sqlite3_bind_text(stmt, 1, experiment.c_str(), -1, SQLITE_STATIC);
	if (rc != SQLITE_OK) {
		log->error(ERR_GSQLITEERROR, "Error binding experiment: >", experiment, "<, ", sqlite3_errmsg(db));
	}
	rc = sqlite3_bind_text(stmt, 2, system_name.c_str(), -1, SQLITE_STATIC);
	if (rc != SQLITE_OK) {
		log->error(ERR_GSQLITEERROR, "Error binding system name: >", system_name, "<, ", sqlite3_errmsg(db));
	}
	rc = sqlite3_bind_text(stmt, 3, variation.c_str(), -1, SQLITE_STATIC);
	if (rc != SQLITE_OK) {
		log->error(ERR_GSQLITEERROR, "Error binding variation: >", variation, "<, ", sqlite3_errmsg(db));
	}
	rc = sqlite3_bind_int(stmt, 4, runno);
	if (rc != SQLITE_OK) {
		log->error(ERR_GSQLITEERROR, "Error binding run number: >", runno, "<, ", sqlite3_errmsg(db));
	}

	if (auto sql = sqlite3_expanded_sql(stmt)) { // returns char*
		log->info(2, sql);
		sqlite3_free(sql); // need to free the expanded SQL string
	}

	vector<string> gvolumePars;
	while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
		int colCount = sqlite3_column_count(stmt);
		for (int i = 0; i < colCount; i++) {
			const char*          colName = sqlite3_column_name(stmt, i);
			const unsigned char* colText = sqlite3_column_text(stmt, i);

			log->info(2, "<sqlite> column: ", (colName ? colName : "NULL"), " = ",
			          (colText ? reinterpret_cast<const char*>(colText) : "NULL"), " (column ", i, ")");

			// Assuming that the first five columns are metadata; additional columns are used for gvolume parameters.
			if (i > 4) {
				colText = sqlite3_column_text(stmt, i);
				gvolumePars.emplace_back(colText ? reinterpret_cast<const char*>(colText) : "");
			}
		}
		system->addGVolume(gvolumePars);
		gvolumePars.clear();
	}

	if (rc != SQLITE_DONE) {
		log->error(ERR_GSQLITEERROR, "Sqlite database error in loadGeometry: ",
		           sqlite3_errmsg(db), " (", rc, ")");
	}

	sqlite3_finalize(stmt);
}
