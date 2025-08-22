#include "FordFactory.h"

#include <iostream>
using namespace std;

void Ford::go() {
	cout << " Factory car ford fordVar[1] " << fordVar[1] << std::endl;
}

Ford::Ford(const std::shared_ptr<GOptions>& gopts) : Car(gopts) {
	cout << " Instantiating Ford" << endl;
	fordVar[0] = 100;
	fordVar[1] = 200;
}
