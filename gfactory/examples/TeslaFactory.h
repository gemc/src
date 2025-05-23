#pragma once

#include "Car.h"

class Tesla : public Car
{
public:
	Tesla();
	void go();
	double autopilot[2][2];
};

extern "C" Car* CarFactory(void) {
	return static_cast<Car*>(new Tesla);
}

