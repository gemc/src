#include "FordFactory.h"

#include <iostream>
using namespace std;

void Ford::go() {
	cout << " Factory car ford fordVar[1] " << fordVar[1] << std::endl;
}

Ford::Ford() {
	cout << " Instantiating Ford" << endl;
	fordVar[0] = 100;
	fordVar[1] = 200;
}
