#pragma once

#include "Car.h"

class Tesla : public Car {
public:
	// inherit the base (const std::shared_ptr<GOptions>&) ctor
	using Car::Car;

	Tesla(const std::shared_ptr<GOptions>&);
	void   go();
	double autopilot[2][2];
};

extern "C" Car* CarFactory(const std::shared_ptr<GOptions>& g) { return static_cast<Car*>(new Tesla(g)); }
