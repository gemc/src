// gtouchable
#include "gtouchable.h"

// c++
#include <iostream>
using namespace std;


int main(int argc, char* argv[])
{

	vector<GIdentifier> identity;

	identity.push_back(GIdentifier("sector", 2));
	identity.push_back(GIdentifier("paddle", 22));

	// a touchable
	// GTouchable ctof(readout, identity, true);

	// cout << " ctof energy multiplier: " << ctof.getEnergyMultiplier() << endl;


    return EXIT_SUCCESS;
}
