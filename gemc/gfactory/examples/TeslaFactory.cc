#include "TeslaFactory.h"

#include <iostream>
using namespace std;

// See header for API docs.

void Tesla::go() {
	cout << " Instantiating Tesla" << endl;

	// Print example state to demonstrate that the object is fully constructed and callable.
	cout << " Factory car tesla autopilot[0][0] " << autopilot[0][0] << endl;
	cout << " Factory car tesla autopilot[0][1] " << autopilot[0][1] << endl;
	cout << " Factory car tesla autopilot[1][0] " << autopilot[1][0] << endl;
	cout << " Factory car tesla autopilot[1][1] " << autopilot[1][1] << endl;

	// Print inherited base-class state.
	cout << " generalCarVar is " << generalCarVar << endl;
}

Tesla::Tesla(const std::shared_ptr<GOptions>& gopts) : Car(gopts) {
	// Example initialization of Tesla-specific state.
	autopilot[0][0] = 0;
	autopilot[0][1] = 1;
	autopilot[1][0] = 10;
	autopilot[1][1] = 11;
}
