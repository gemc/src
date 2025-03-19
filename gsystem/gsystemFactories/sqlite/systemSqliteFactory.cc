// string for gexit
#include "gutilities.h"

using namespace gutilities;

// gsystem
#include "systemSqliteFactory.h"

// c++
using namespace std;


GSystemSQLiteFactory::GSystemSQLiteFactory() {

}


void GSystemSQLiteFactory::initialize_sqlite_db(GSystem *system, int verbosity) {
	// Save parameters from the system.
	system_name = system->getName();
	variation = system->getVariation();
	runno = system->getRunno();

	// Use system's dbhost if not already set.
	if (dbhost == "na") {
		dbhost = system->get_dbhost();
	}

	// Attempt to open the primary database file.
	int rc = sqlite3_open_v2(dbhost.c_str(), &db, SQLITE_OPEN_READWRITE, NULL);
	if (rc != SQLITE_OK) {
		// If opening failed, close the handle (if non-null) and set it to nullptr.
		if (db) {
			sqlite3_close(db);
			db = nullptr;
		}
		// Try alternative locations from possibleLocationOfFiles.
		for (auto trialLocation : possibleLocationOfFiles) {
			string newName = trialLocation + "/" + dbhost;
			if (verbosity == GVERBOSITY_DETAILS) {
				cout << GSYSTEMLOGHEADER << "Trying sqlite file " << newName << endl;
			}
			rc = sqlite3_open_v2(newName.c_str(), &db, SQLITE_OPEN_READWRITE, NULL);
			if (rc == SQLITE_OK) {
				if (verbosity >= GVERBOSITY_SUMMARY) {
					cout << GSYSTEMLOGHEADER << "Sqlite file >" << KGRN << newName << RST << "< opened successfully" << endl;
				}
				return;
			} else {
				// Close and reset db if open failed.
				if (db) {
					sqlite3_close(db);
					db = nullptr;
				}
			}
		}
	} else {
		cout << GSYSTEMLOGHEADER << "Sqlite file >" << KGRN << dbhost << RST << "< opened successfully" << endl;
		return;
	}

	// If we are here, no valid database file was found.
	cerr << GSYSTEMLOGHEADER << "Sqlite database >" << dbhost << "< not found" << endl;
	gexit(EC__GSETUPFILENOTOFOUND);
}


void GSystemSQLiteFactory::closeSystem() {

	if (db) {
		sqlite3_close(db);
		db = nullptr;  // Reset the pointer after closing
	}
	possibleLocationOfFiles.clear();
    cout << GSYSTEMLOGHEADER << "Sqlite database >" << KGRN << dbhost << RST << "<  closed successfully" << endl;

}
