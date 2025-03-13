// gsystem
#include "systemSqliteFactory.h"

// c++
using namespace std;


void GSystemSQLiteFactory::loadMaterials(GSystem *system, int verbosity) {
	if (db == nullptr) {
		initialize_sqlite_db(system, verbosity);
	}

	const char *sql_query = "SELECT DISTINCT * FROM materials WHERE system = ? AND variation = ? AND run = ?";

	sqlite3_stmt *stmt;
	int rc = sqlite3_prepare_v2(db, sql_query, -1, &stmt, NULL);
	if (rc != SQLITE_OK) {
		cerr << GSYSTEMLOGHEADER << "Sqlite database error:" << sqlite3_errmsg(db) << endl;
		gexit(EC__GSQLITEERROR);
	}

	// Bind parameters (assuming system, variation, and runno are available as strings)
	string system_name = system->getName();
	string variation = system->getVariation();
	int runno = system->getRunno();

	sqlite3_bind_text(stmt, 1, system_name.c_str(), -1, SQLITE_STATIC);
	sqlite3_bind_text(stmt, 2, variation.c_str(), -1, SQLITE_STATIC);
	sqlite3_bind_int(stmt, 3, runno);

	vector<string> gmaterialPars;
	while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
		// Looping over columns
		for (int i = 0; i < sqlite3_column_count(stmt); i++) {
			if (verbosity >= GVERBOSITY_DETAILS) {
				cout << GSYSTEMLOGHEADER << "<sqlite> column: " << KRED << sqlite3_column_name(stmt, i)
					 << " = " << sqlite3_column_text(stmt, i) << RST << " (column " << i << ")" << endl;
			}
			// Matching gmaterial constructor order, after experiment, system, variation, run
			if (i > 4) { gmaterialPars.push_back((char *)sqlite3_column_text(stmt, i)); }
		}
		system->addGMaterial(gmaterialPars, verbosity);
		gmaterialPars.clear();
		if (verbosity >= GVERBOSITY_DETAILS) { cout << endl; }
	}
	if (rc != SQLITE_DONE) {
		cerr << GSYSTEMLOGHEADER << "Sqlite database error:" << sqlite3_errmsg(db) << endl;
		gexit(EC__GSQLITEERROR);
	}
	sqlite3_finalize(stmt);
}
