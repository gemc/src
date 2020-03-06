// c++
#include <iostream>
using namespace std;

// goptions
#include "goptions.h"

// utilities, options definitions
#include "utilities.h"



int main(int argc, char* argv[])
{
	// the option are loaded in utilities/defineOptions.cc
	// they include the gemc core options and any frameworks options
	GOptions *gopts = new GOptions(argc, argv, defineOptions());


	bool gui = getGui(gopts);
	createQtApplication(argc, argv, gui);


	cout << gui << endl;

	return 1;
}



