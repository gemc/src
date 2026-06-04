/**
 * \file loadMaterials.cc
 * \brief Materials loading implementation for GSystemSQLiteFactory.
 *
 * See systemSqliteFactory.h for API docs.
 */

// gsystem
#include "systemSqliteFactory.h"
#include "gsystemConventions.h"

// c++
using namespace std;


void GSystemSQLiteFactory::loadMaterials(GSystem* system) {
	// skip ROOT system
	if (system->getName() == ROOTWORLDGVOLUMENAME) { return; }

	// Initialize the DB if needed.
	if (db == nullptr) { initialize_sqlite_db(system); }

	// Check that db is valid.
	if (db == nullptr) { log->error(ERR_GSQLITEERROR, "Database pointer is still null after initialization."); }

	// Check if the materials table has any rows.
	// An empty materials table can be valid if materials come from the Geant4 database.
	int           count       = 0;
	const char*   count_query = "SELECT COUNT(*) FROM materials";
	sqlite3_stmt* count_stmt  = nullptr;
	int           rc          = sqlite3_prepare_v2(db, count_query, -1, &count_stmt, nullptr);
	if (rc != SQLITE_OK) {
		log->error(ERR_GSQLITEERROR, "Sqlite error preparing count query in loadMaterials: ",
		           sqlite3_errmsg(db), " (", rc, ") using query: ", count_query);
	}
	if (sqlite3_step(count_stmt) == SQLITE_ROW) { count = sqlite3_column_int(count_stmt, 0); }
	sqlite3_finalize(count_stmt);

	if (count == 0) {
		log->info(2, "Table 'materials' is empty for system <", system_name, ">, variation <", variation, ">, "
		          "run ", runno, ". This may be ok if the materials are from the Geant4 database.");
		return;
	}


	// Prepare the SQL query.
	const char*   sql_query = "SELECT DISTINCT * FROM materials WHERE system = ? AND variation = ? AND run = ?";
	sqlite3_stmt* stmt      = nullptr;
	rc                      = sqlite3_prepare_v2(db, sql_query, -1, &stmt, nullptr);
	if (rc != SQLITE_OK) {
		log->error(ERR_GSQLITEERROR, "Sqlite database error in loadMaterials: ",
		           sqlite3_errmsg(db), " (", rc, ") using query: ", sql_query);
	}

	// Bind parameters.
	string system_name = system->getName();
	string variation   = system->getVariation();
	int    runno       = system->getRunno();

	sqlite3_bind_text(stmt, 1, system_name.c_str(), -1, SQLITE_STATIC);
	sqlite3_bind_text(stmt, 2, variation.c_str(), -1, SQLITE_STATIC);
	sqlite3_bind_int(stmt, 3, runno);


	vector<string> gmaterialPars;
	while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
		const int ncols = sqlite3_column_count(stmt);
		for (int i = 0; i < ncols; ++i) {
			const char* cname = sqlite3_column_name(stmt, i);

			// Safely get text (use a literal "NULL" for SQLITE_NULL)
			const char* ctext =
				(sqlite3_column_type(stmt, i) == SQLITE_NULL)
					? "NULL"
					: reinterpret_cast<const char*>(sqlite3_column_text(stmt, i));

			log->info(2, "<sqlite> column: ", cname, " = ", ctext);

			// The first columns are metadata; columns beyond index 4 are GMaterial constructor parameters.
			if (i > 4) {
				// Push a string even for NULLs to avoid nullptr UB.
				gmaterialPars.emplace_back(ctext); // will be "NULL" string for NULL columns
			}
		}
		system->addGMaterial(gmaterialPars);
		gmaterialPars.clear();
	}


	if (rc != SQLITE_DONE) {
		log->error(ERR_GSQLITEERROR, "Sqlite database error in loadMaterials: ",
		           sqlite3_errmsg(db), " (", rc, ")");
	}


	sqlite3_finalize(stmt);
}
