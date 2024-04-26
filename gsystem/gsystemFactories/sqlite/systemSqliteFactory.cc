// string for gexit
#include "gutilities.h"
using namespace gutilities;

// gsystem
#include "systemSqliteFactory.h"

// c++
using namespace std;



GSystemSQLiteFactory::GSystemSQLiteFactory() {
	
}



void GSystemSQLiteFactory::initialize_sqlite_db(GSystem *system, int verbosity)
{
    system_name = system->getName();
    variation   = system->getVariation();
    runno       = system->getRunno();
    sqlite_file = system->getSqliteFile();

    // sqlite3_open_v2 will return SQLITE_OK (0) on success if the file exist (note: even if it's not a sqlite db)
    int rc = sqlite3_open_v2(sqlite_file.c_str(), &db, SQLITE_OPEN_READWRITE, NULL);

    if( rc ) {

        rc = 0;
        sqlite3_close(db);
        // file not found, now trying other locations
        for(auto trialLocation : possibleLocationOfFiles) {

            string newName = trialLocation + "/" + sqlite_file;
            if( verbosity == GVERBOSITY_DETAILS ) {
                cout << GSYSTEMLOGHEADER << "Trying sqlite file " << newName << endl;
            }
            rc = sqlite3_open_v2(newName.c_str(), &db, SQLITE_OPEN_READWRITE, NULL);

            if( rc == 0 ) {
                if(verbosity >= GVERBOSITY_SUMMARY) {
                    cout << GSYSTEMLOGHEADER << "Sqlite file >" <<  KMAG << newName << RST  << "< opened successfully" << endl;
                }
                return;
            }
        }
    } else {
        cout << GSYSTEMLOGHEADER << "Sqlite file >" <<  KMAG << sqlite_file << RST  << "< opened successfully" << endl;
        return;
    }

    // If we are here, the file was not found. Exit
    cerr << GSYSTEMLOGHEADER << "Sqlite database " << sqlite_file << " not found " << endl;
    gexit(EC__GSETUPFILENOTOFOUND);
}

void GSystemSQLiteFactory::closeSystem() {
    sqlite3_close(db);
    possibleLocationOfFiles.clear();
    cout << GSYSTEMLOGHEADER << "Sqlite database >" <<  KMAG << sqlite_file << RST  << "<  closed successfully" << endl;

}
