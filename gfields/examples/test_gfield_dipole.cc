// gfields
#include "gfield_options.h"
#include "gmagneto.h"

// c++
#include <iostream>

using namespace std;

int main(int argc, char* argv[]) {
	// Initialize GOptions (parsed from YAML or another source)
	auto gopts = std::make_shared<GOptions>(argc, argv, gfields::defineOptions());

	// Create a GMagneto instance to manage fields
	auto magneto = std::make_shared<GMagneto>(gopts);

	    string field_name = "dipole";

	    // Check if a specific field exists
	    if (magneto->isField(field_name)) {
	    	auto dipole_field = magneto->getField(field_name);
	    	auto dipole_field_manager = magneto->getFieldMgr(field_name);

	    	// pos is a double[3] representing x,y,z
	    	// iterate through 100 pos that span random positions between 0 and 100 in each coordinate
	    	for (int i = 0; i < 100; i++) {
	    		double x = rand() % 100;
	    		double y = rand() % 100;
	    		double z = rand() % 100;

	    		double pos[3] = {x, y, z};
	    		double bfield[3];
	    		dipole_field->GetFieldValue(pos, bfield);

	    	}


	    } else {
	        cout << "Field " << field_name << " was not found." << endl;
	    }

	return EXIT_SUCCESS;
}
