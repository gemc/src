#include "TeslaFactory.h"

#include <iostream>
using namespace std;

void Tesla::go() {
	cout << " Instantiating Tesla" << endl;
	cout << " Factory car tesla autopilot[0][0] " << autopilot[0][0] << endl;
	cout << " Factory car tesla autopilot[0][1] " << autopilot[0][1] << endl;
	cout << " Factory car tesla autopilot[1][0] " << autopilot[1][0] << endl;
	cout << " Factory car tesla autopilot[1][1] " << autopilot[1][1] << endl;

	cout << " generalCarVar is " << generalCarVar << endl;
}

Tesla::Tesla(const std::shared_ptr<GOptions>& gopts) : Car(gopts) {
	autopilot[0][0] = 0;
	autopilot[0][1] = 1;
	autopilot[1][0] = 10;
	autopilot[1][1] = 11;
}
