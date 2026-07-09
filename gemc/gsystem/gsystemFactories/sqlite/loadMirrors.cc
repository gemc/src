/**
 * \file loadMirrors.cc
 * \brief Mirrors loading implementation for GSystemSQLiteFactory.
 *
 * See systemSqliteFactory.h for API docs.
 */

// gsystem
#include "systemSqliteFactory.h"
#include "gsystemConventions.h"

namespace {
bool mirrors_column_exists(sqlite3* db, const std::string& column_name) {
	sqlite3_stmt* stmt = nullptr;
	int rc = sqlite3_prepare_v2(db, "SELECT name FROM PRAGMA_TABLE_INFO('mirrors')", -1, &stmt, nullptr);
	if (rc != SQLITE_OK) { return false; }

	bool exists = false;
	while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
		const unsigned char* colText = sqlite3_column_text(stmt, 0);
		if (colText != nullptr && column_name == reinterpret_cast<const char*>(colText)) {
			exists = true;
			break;
		}
	}

	sqlite3_finalize(stmt);
	return exists;
}
}

void GSystemSQLiteFactory::loadMirrors(GSystem* system) {
	// skip ROOT system
	if (system->getName() == ROOTWORLDGVOLUMENAME) { return; }

	// Initialize the DB if needed.
	if (db == nullptr) { initialize_sqlite_db(system); }

	// Check that db is valid.
	if (db == nullptr) { log->error(ERR_GSQLITEERROR, "Database pointer is still null after initialization."); }

	// Databases created before mirror support may lack the table, and databases whose
	// systems define no mirrors carry a bare table with only the id column: mirrors
	// are optional in both cases.
	if (!table_exists(db, "mirrors") || !mirrors_column_exists(db, "name")) {
		log->info(2, "No usable 'mirrors' table in the database. This is ok if no system defines mirrors.");
		return;
	}

	// Columns are selected explicitly by name (in the canonical GMirror parameter order)
	// so the loader does not depend on the table column order.
	const std::string transmittance_column = mirrors_column_exists(db, "transmittance")
		                                       ? "transmittance"
		                                       : "NULL AS transmittance";
	const std::string sql_query =
		"SELECT name, description, type, finish, model, border, matOptProps, photonEnergy, "
		"indexOfRefraction, reflectivity, efficiency, specularlobe, specularspike, backscatter, "
		+ transmittance_column +
		", sigmaAlpha FROM mirrors WHERE experiment = ? AND system = ? AND variation = ? AND run = ?";
	sqlite3_stmt* stmt = nullptr;
	int           rc   = sqlite3_prepare_v2(db, sql_query.c_str(), -1, &stmt, nullptr);
	if (rc != SQLITE_OK) {
		log->error(ERR_GSQLITEERROR, "Sqlite error preparing query in loadMirrors: ",
		           sqlite3_errmsg(db), " (", rc, ") using query: ", sql_query);
	}

	// Bind parameters.
	std::string experiment  = system->getExperiment();
	std::string system_name = system->getName();
	std::string variation   = system->getVariation();
	int         runno       = system->getRunno();

	sqlite3_bind_text(stmt, 1, experiment.c_str(), -1, SQLITE_STATIC);
	sqlite3_bind_text(stmt, 2, system_name.c_str(), -1, SQLITE_STATIC);
	sqlite3_bind_text(stmt, 3, variation.c_str(), -1, SQLITE_STATIC);
	sqlite3_bind_int(stmt, 4, runno);

	// Log the expanded SQL for debugging (caller must free it).
	if (auto sql = sqlite3_expanded_sql(stmt)) {
		log->info(2, sql);
		sqlite3_free(sql);
	}

	std::vector<std::string> gmirrorPars;
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

			gmirrorPars.emplace_back(ctext);
		}
		system->addGMirror(gmirrorPars);
		gmirrorPars.clear();
	}

	if (rc != SQLITE_DONE) {
		log->error(ERR_GSQLITEERROR, "Sqlite database error in loadMirrors: ",
		           sqlite3_errmsg(db), " (", rc, ")");
	}

	sqlite3_finalize(stmt);
}
