#pragma once

#include "Car.h"

class Ford : public Car
{
public:
	// inherit the base (const std::shared_ptr<GOptions>&) ctor
	using Car::Car;

	Ford(const std::shared_ptr<GOptions>&);

	void go();
	double fordVar[2]{};
};

extern "C" Car* CarFactory(const std::shared_ptr<GOptions>& g) {
	return static_cast<Car*>(new Ford(g));
}
