// gsystem
#include "systemSqliteFactory.h"

// c++
using namespace std;


void GSystemSQLiteFactory::loadMaterials(GSystem *system, int verbosity) {
	// Initialize the DB if needed.
	if (db == nullptr) {
		initialize_sqlite_db(system, verbosity);
	}

	// Check that db is valid.
	if (db == nullptr) {
		cerr << GSYSTEMLOGHEADER << "Error: Database pointer is still null after initialization." << endl;
		gexit(EC__GSQLITEERROR);
	}

	// Check if the materials table has any rows
	int count = 0;
	const char *count_query = "SELECT COUNT(*) FROM materials";
	sqlite3_stmt *count_stmt = nullptr;
	int rc = sqlite3_prepare_v2(db, count_query, -1, &count_stmt, NULL);
	if (rc != SQLITE_OK) {
		cerr << GSYSTEMLOGHEADER << "Sqlite error preparing count query in loadMaterials: "
			 << sqlite3_errmsg(db) << " (" << rc << ") using query: " << count_query << endl;
		gexit(EC__GSQLITEERROR);
	}
	if (sqlite3_step(count_stmt) == SQLITE_ROW) {
		count = sqlite3_column_int(count_stmt, 0);
	}
	sqlite3_finalize(count_stmt);

	if (count == 0) {
		if (verbosity >= GVERBOSITY_DETAILS) {
			cout << GSYSTEMLOGHEADER << "Table 'materials' is empty for system <"
				 << system_name << ">, variation <" << variation << ">, run " << runno << ". Returning." << endl;
		}
		return;
	}


	// Prepare the SQL query.
	const char *sql_query = "SELECT DISTINCT * FROM materials WHERE system = ? AND variation = ? AND run = ?";
	sqlite3_stmt *stmt = nullptr;
	rc = sqlite3_prepare_v2(db, sql_query, -1, &stmt, NULL);
	if (rc != SQLITE_OK) {
		cerr << GSYSTEMLOGHEADER << "Sqlite database error in loadMaterials: "
			 << sqlite3_errmsg(db) << " (" << rc << ") using query: " << sql_query << endl;
		gexit(EC__GSQLITEERROR);
	}

	// Bind parameters.
	string system_name = system->getName();
	string variation = system->getVariation();
	int runno = system->getRunno();

	sqlite3_bind_text(stmt, 1, system_name.c_str(), -1, SQLITE_STATIC);
	sqlite3_bind_text(stmt, 2, variation.c_str(), -1, SQLITE_STATIC);
	sqlite3_bind_int(stmt, 3, runno);

	vector<string> gmaterialPars;
	while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
		for (int i = 0; i < sqlite3_column_count(stmt); i++) {
			if (verbosity >= GVERBOSITY_DETAILS) {
				cout << GSYSTEMLOGHEADER << "<sqlite> column: " << KRED
					 << sqlite3_column_name(stmt, i) << " = "
					 << sqlite3_column_text(stmt, i) << RST << " (column " << i << ")" << endl;
			}
			// Assuming gmaterial constructor expects parameters beyond the first 4 columns.
			if (i > 4) {
				gmaterialPars.push_back((char *)sqlite3_column_text(stmt, i));
			}
		}
		system->addGMaterial(gmaterialPars, verbosity);
		gmaterialPars.clear();
		if (verbosity >= GVERBOSITY_DETAILS) { cout << endl; }
	}
	if (rc != SQLITE_DONE) {
		cerr << GSYSTEMLOGHEADER << "Sqlite database error in loadMaterials: "
			 << sqlite3_errmsg(db) << " (rc = " << rc << ")" << endl;
		gexit(EC__GSQLITEERROR);
	}
	sqlite3_finalize(stmt);
}
