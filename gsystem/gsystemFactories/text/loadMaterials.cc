// gsystem
#include "systemTextFactory.h"


void GSystemTextFactory::loadMaterials(GSystem *system, int verbosity) {

    // will continue if not found
    ifstream *IN = gSystemTextFileStream(system, GTEXTMATSTYPE, verbosity);

    if (IN != nullptr) {

        if (verbosity >= GVERBOSITY_SUMMARY) {
            cout << GSYSTEMLOGHEADER << "Loading <text> materials for <" << KWHT << system->getName() << RST << ">"
                 << endl;
        }

        // loading volumes
        while (!IN->eof()) {

            string dbline;
            getline(*IN, dbline);

            if (!dbline.size())
                continue;

            // extract gvolume parameters
            vector <string> gmaterialsPars = gutilities::getStringVectorFromStringWithDelimiter(dbline, "|");
            system->addGMaterial(gmaterialsPars, verbosity);
        }

        IN->close();
    }
}
