// gfields
#include "gfieldOptions.h"
#include "gmagneto.h"

// c++
#include <iostream>

using namespace std;

int main(int argc, char *argv[]) {

    // Initialize GOptions (parsed from YAML or another source)
    GOptions *gopts = new GOptions(argc, argv, gfields::defineOptions());

    // Create a GMagneto instance to manage fields
    GMagneto *magneto = new GMagneto(gopts);

    string field_name = "dipole";

    // Check if a specific field exists
    if (magneto->isField(field_name)) {
//        GField *field = magneto->getField(field_name);
//        G4FieldManager *fieldManager = magneto->getFieldMgr(field_name);
    } else {
        cout << "Field " << field_name << " was not found." << endl;
    }

    return EXIT_SUCCESS;
}
