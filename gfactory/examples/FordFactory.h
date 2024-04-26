#ifndef SECONDFACTORY_H
#define SECONDFACTORY_H 1

#include "Car.h"

class Ford : public Car
{
public:
	Ford();
	void go();
	float fordVar[2];
};

extern "C" Car* CarFactory(void) {
	return static_cast<Car*>(new Ford);
}


#endif // SECONDFACTORY_H
