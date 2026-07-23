/**
 * \file loadMirrors.cc
 * \brief Mirrors loading implementation for GSystemCADFactory.
 *
 * CAD volumes may carry an optical surface (e.g. the LTCC Winston cones name
 * \c ltcc_AlMgF2 through \c GVolume.mirror). The surface definitions live in the
 * same sqlite database as the geometry, so this loader reads the \c mirrors table
 * for the system exactly like the sqlite factory does. See systemCadFactory.h.
 */

// gsystem
#include "systemCadFactory.h"
#include "gsystemConventions.h"

// gemc
#include "gutilities.h"

// sqlite
#include "sqlite3.h"

// c++
#include <vector>

using namespace std;

namespace {
	/// \brief Check whether a table exists in the database.
	bool table_exists(sqlite3* db, const char* name) {
		sqlite3_stmt* stmt = nullptr;
		if (sqlite3_prepare_v2(db, "SELECT 1 FROM sqlite_master WHERE type='table' AND name=?",
		                       -1, &stmt, nullptr) != SQLITE_OK) {
			return false;
		}
		sqlite3_bind_text(stmt, 1, name, -1, SQLITE_TRANSIENT);
		bool exists = (sqlite3_step(stmt) == SQLITE_ROW);
		sqlite3_finalize(stmt);
		return exists;
	}

	/// \brief Check whether a column exists in the \c mirrors table.
	bool mirrors_column_exists(sqlite3* db, const std::string& column_name) {
		sqlite3_stmt* stmt = nullptr;
		if (sqlite3_prepare_v2(db, "SELECT name FROM PRAGMA_TABLE_INFO('mirrors')", -1, &stmt, nullptr) != SQLITE_OK) {
			return false;
		}
		bool exists = false;
		while (sqlite3_step(stmt) == SQLITE_ROW) {
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

void GSystemCADFactory::loadMirrors(GSystem* s) {
	// skip ROOT system
	if (s->getName() == ROOTWORLDGVOLUMENAME) { return; }

	// The mirror definitions come from the sqlite database (same file as the geometry).
	string dbhost = s->get_dbhost();
	if (dbhost.empty() || dbhost == "na") { dbhost = GSYSTEMSQLITETDEFAULTFILE; }

	vector<string> dirs = {
		".",
		gutilities::gemc_root().string(),
		(gutilities::gemc_root() / "examples").string()
	};
	auto dbPath = gutilities::searchForFileInLocations(dirs, dbhost);
	if (!dbPath) {
		log->error(ERR_GSQLITEERROR, "CAD factory: sqlite database <" + dbhost + "> not found.");
		return;
	}

	sqlite3* db = nullptr;
	if (sqlite3_open_v2(dbPath.value().c_str(), &db, SQLITE_OPEN_READONLY, nullptr) != SQLITE_OK) {
		sqlite3_close(db);
		log->error(ERR_GSQLITEERROR, "CAD factory: failed to open sqlite database <" + dbhost + ">.");
		return;
	}

	// Mirrors are optional: databases created before mirror support may lack the table, and systems
	// that define no mirrors carry a bare table with only the id column.
	if (!table_exists(db, "mirrors") || !mirrors_column_exists(db, "name")) {
		log->info(2, "CAD factory: no usable 'mirrors' table (ok if the system defines no mirrors).");
		sqlite3_close(db);
		return;
	}

	// Columns are selected explicitly by name (canonical GMirror parameter order) so the loader does
	// not depend on the table column order.
	const string transmittance_column = mirrors_column_exists(db, "transmittance")
	                                     ? "transmittance"
	                                     : "NULL AS transmittance";
	const string sql_query =
		"SELECT name, description, type, finish, model, border, matOptProps, photonEnergy, "
		"indexOfRefraction, reflectivity, efficiency, specularlobe, specularspike, backscatter, "
		+ transmittance_column +
		", sigmaAlpha FROM mirrors WHERE experiment = ? AND system = ? AND variation = ? AND run = ?";

	sqlite3_stmt* stmt = nullptr;
	if (sqlite3_prepare_v2(db, sql_query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
		log->error(ERR_GSQLITEERROR, "CAD factory: error preparing mirrors query: ", sqlite3_errmsg(db));
		sqlite3_close(db);
		return;
	}

	string experiment  = s->getExperiment();
	string system_name = s->getName();
	string variation   = s->getVariation();
	int    runno       = s->getRunno();

	sqlite3_bind_text(stmt, 1, experiment.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt, 2, system_name.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_text(stmt, 3, variation.c_str(), -1, SQLITE_TRANSIENT);
	sqlite3_bind_int(stmt, 4, runno);

	int rc;
	while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
		vector<string> gmirrorPars;
		const int      ncols = sqlite3_column_count(stmt);
		for (int i = 0; i < ncols; ++i) {
			const char* ctext =
				(sqlite3_column_type(stmt, i) == SQLITE_NULL)
					? "NULL"
					: reinterpret_cast<const char*>(sqlite3_column_text(stmt, i));
			gmirrorPars.emplace_back(ctext);
		}
		s->addGMirror(gmirrorPars);
	}

	if (rc != SQLITE_DONE) {
		log->error(ERR_GSQLITEERROR, "CAD factory: sqlite error while reading mirrors: ", sqlite3_errmsg(db));
	}

	sqlite3_finalize(stmt);
	sqlite3_close(db);
}
