#pragma once

#include "Car.h"

class Ford : public Car
{
public:
	Ford();
	void go();
	double fordVar[2]{};
};

extern "C" Car* CarFactory(void) {
	return static_cast<Car*>(new Ford);
}
