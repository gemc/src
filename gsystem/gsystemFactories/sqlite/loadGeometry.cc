// gsystem
#include "systemSqliteFactory.h"

// glibrary
#include "gutilities.h"

// c++
#include <iostream>

using namespace std;

void GSystemSQLiteFactory::loadGeometry(GSystem *system, int verbosity) {
	// Initialize the database if not already done.
	if (db == nullptr) {
		initialize_sqlite_db(system, verbosity);
	}
	if (db == nullptr) {
		cerr << GSYSTEMLOGHEADER << "Error: db pointer is null after initialization." << endl;
		gexit(EC__GSQLITEERROR);
	}

	const char *sql_query = "SELECT DISTINCT * FROM geometry WHERE system = ? AND variation = ? AND run = ?";
	sqlite3_stmt *stmt = nullptr;
	int rc = sqlite3_prepare_v2(db, sql_query, -1, &stmt, NULL);
	if (rc != SQLITE_OK) {
		cerr << GSYSTEMLOGHEADER << "Sqlite database error in loadGeometry: "
			 << sqlite3_errmsg(db) << " using query: " << sql_query << endl;
		gexit(EC__GSQLITEERROR);
	}

	// Bind parameters.
	string system_name = system->getName();
	string variation = system->getVariation();
	int runno = system->getRunno();

	rc = sqlite3_bind_text(stmt, 1, system_name.c_str(), -1, SQLITE_STATIC);
	if (rc != SQLITE_OK) {
		cerr << GSYSTEMLOGHEADER << "Error binding system name: " << sqlite3_errmsg(db) << endl;
		gexit(EC__GSQLITEERROR);
	}
	rc = sqlite3_bind_text(stmt, 2, variation.c_str(), -1, SQLITE_STATIC);
	if (rc != SQLITE_OK) {
		cerr << GSYSTEMLOGHEADER << "Error binding variation: " << sqlite3_errmsg(db) << endl;
		gexit(EC__GSQLITEERROR);
	}
	rc = sqlite3_bind_int(stmt, 3, runno);
	if (rc != SQLITE_OK) {
		cerr << GSYSTEMLOGHEADER << "Error binding run number: " << sqlite3_errmsg(db) << endl;
		gexit(EC__GSQLITEERROR);
	}

	vector<string> gvolumePars;
	while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
		int colCount = sqlite3_column_count(stmt);
		for (int i = 0; i < colCount; i++) {
			if (verbosity >= GVERBOSITY_DETAILS) {
				const char *colName = sqlite3_column_name(stmt, i);
				const unsigned char *colText = sqlite3_column_text(stmt, i);
				cout << GSYSTEMLOGHEADER << "<sqlite> column: "
					 << KRED << (colName ? colName : "NULL")
					 << " = " << (colText ? reinterpret_cast<const char*>(colText) : "NULL")
					 << RST << " (column " << i << ")" << endl;
			}
			// Assuming that the first five columns are metadata; additional columns are used for gvolume parameters.
			if (i > 4) {
				const unsigned char *colText = sqlite3_column_text(stmt, i);
				gvolumePars.push_back(colText ? reinterpret_cast<const char*>(colText) : "");
			}
		}
		system->addGVolume(gvolumePars, verbosity);
		gvolumePars.clear();
		if (verbosity >= GVERBOSITY_DETAILS) { cout << endl; }
	}

	if (rc != SQLITE_DONE) {
		cerr << GSYSTEMLOGHEADER << "Sqlite database error in loadGeometry: "
			 << sqlite3_errmsg(db) << " (rc = " << rc << ")" << endl;
		gexit(EC__GSQLITEERROR);
	}

	sqlite3_finalize(stmt);
}
