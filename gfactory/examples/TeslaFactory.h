#ifndef SECONDFACTORY_H
#define SECONDFACTORY_H 1

#include "Car.h"

class Tesla : public Car
{
public:
	Tesla();
	void go();
	float autopilot[2][2];
};

extern "C" Car* CarFactory(void) {
	return static_cast<Car*>(new Tesla);
}


#endif // SECONDFACTORY_H
