// gsystem
#include "systemSqliteFactory.h"

// glibrary
#include "gutilities.h"

// c++
#include <iostream>

using namespace std;


void GSystemSQLiteFactory::loadGeometry(GSystem *system, int verbosity) {
    if (db == nullptr) {
        initialize_sqlite_db(system, verbosity);
    }

    string sql_query = "SELECT * from geometry ;";
    //where system = '" + system_name;
//    sql_query += "' and variation = '" + variation ;
//    sql_query += "' and runno = " + to_string(runno) + ";";


    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, sql_query.c_str(), -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        cerr << GSYSTEMLOGHEADER << "Sqlite database error:" << sqlite3_errmsg(db) << endl;
        gexit(EC__GSQLITEERROR);
    }
    vector <string> gvolumePars;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {

        // looping over columns
        for (int i = 0; i < sqlite3_column_count(stmt); i++) {
            if (verbosity >= GVERBOSITY_DETAILS) {
                cout << GSYSTEMLOGHEADER << "<sqlite> column: " << KRED << sqlite3_column_name(stmt, i)
                     << " = " << sqlite3_column_text(stmt, i) << RST << " (column " << i << ")" << endl;
            }
            // matching gvolume constructor order
            if (i > 3) { gvolumePars.push_back((char *) sqlite3_column_text(stmt, i)); }
        }

        system->addGVolume(gvolumePars, verbosity);
        gvolumePars.clear();
        if (verbosity >= GVERBOSITY_DETAILS) { cout << endl; }
    }
    if (rc != SQLITE_DONE) {
        cerr << GSYSTEMLOGHEADER << "Sqlite database error:" << sqlite3_errmsg(db) << endl;
        gexit(EC__GSQLITEERROR);
    }
    sqlite3_finalize(stmt);

}

