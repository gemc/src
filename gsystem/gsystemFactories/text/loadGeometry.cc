// gsystem
#include "systemTextFactory.h"

// glibrary
#include "gutilities.h"

// c++
#include <iostream>

void GSystemTextFactory::loadGeometry(GSystem *system, int verbosity) {
    // will exit if not found
    ifstream *IN = gSystemTextFileStream(system, GTEXTGEOMTYPE, verbosity);

    if (IN != nullptr) {

        if (verbosity >= GVERBOSITY_SUMMARY) {
            cout << GSYSTEMLOGHEADER << "Loading <text> geometry for <" << KWHT << system->getName() << RST << ">"
                 << endl;
        }

        // loading volumes
        while (!IN->eof()) {

            string dbline;
            getline(*IN, dbline);

            if (!dbline.size())
                continue;

            // extract gvolume parameters
            vector <string> gvolumePars = gutilities::getStringVectorFromStringWithDelimiter(dbline, "|");
            system->addGVolume(gvolumePars, verbosity);
        }

        IN->close();
    }
}

