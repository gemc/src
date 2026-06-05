/**
 * \file loadGeometry.cc
 * \brief Geometry loading implementation for GSystemSQLiteFactory.
 *
 * See systemSqliteFactory.h for API docs.
 */

// gsystem
#include "systemSqliteFactory.h"
#include "gsystemConventions.h"

namespace {
	bool geometry_column_exists(sqlite3* db, const std::string& column_name) {
		sqlite3_stmt* stmt = nullptr;
		int rc = sqlite3_prepare_v2(db, "SELECT name FROM PRAGMA_TABLE_INFO('geometry')", -1, &stmt, nullptr);
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

void GSystemSQLiteFactory::loadGeometry(GSystem* system) {
	// skip ROOT system
	if (system->getName() == ROOTWORLDGVOLUMENAME) { return; }

	// Initialize the DB if needed.
	if (db == nullptr) { initialize_sqlite_db(system); }

	// Check that db is valid.
	if (db == nullptr) { log->error(ERR_GSQLITEERROR, "Database pointer is still null after initialization."); }


	const std::string placement_column = geometry_column_exists(db, "g4placement_type")
	                                   ? "g4placement_type"
	                                   : "'" DEFAULTG4PLACEMENTTYPE "' AS g4placement_type";
	const std::string sql_query =
		"SELECT DISTINCT name, solid, parameters, material, mother, position, rotations, " +
		placement_column +
		", mfield, visible, style, color, opacity, digitization, identifier, copyOf, solidsOpr, mirror, "
		"exist, description FROM geometry WHERE experiment = ? AND system = ? AND variation = ? AND run = ?";
	sqlite3_stmt* stmt = nullptr;
	int           rc   = sqlite3_prepare_v2(db, sql_query.c_str(), -1, &stmt, nullptr);
	if (rc != SQLITE_OK) {
		log->error(ERR_GSQLITEERROR, "Sqlite error preparing count query in loadGeometry: ",
		           sqlite3_errmsg(db), " (", rc, ") using query: ", sql_query);
	}

	// Bind parameters.
	std::string experiment  = system->getExperiment();
	std::string system_name = system->getName();
	std::string variation   = system->getVariation();
	int         runno       = system->getRunno();

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

	// Log the expanded SQL for debugging (caller must free it).
	if (auto sql = sqlite3_expanded_sql(stmt)) {
		// returns char*
		log->info(2, sql);
		sqlite3_free(sql); // need to free the expanded SQL string
	}

	std::vector<std::string> gvolumePars;
	while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
		int colCount = sqlite3_column_count(stmt);
		for (int i = 0; i < colCount; i++) {
			const char*          colName = sqlite3_column_name(stmt, i);
			const unsigned char* colText = sqlite3_column_text(stmt, i);

			log->info(2, "<sqlite> column: ", (colName ? colName : "NULL"), " = ",
			          (colText ? reinterpret_cast<const char*>(colText) : "NULL"), " (column ", i, ")");

			gvolumePars.emplace_back(colText ? reinterpret_cast<const char*>(colText) : "");
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
