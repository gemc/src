#include "gfactory.h"
#include "ShapeFactory.h"
#include "Car.h"

#include <iostream>
using namespace std;

// managerA: loads known classes Triangle and Box from the base Shape, in ShapeFactory.h
// managerB: loads DLL that instantiate derived factories - we only know of the base class, in this case "Car"

int main()
{

	// AV manages Shapes. It's the same as managerA but with verbosity 1
	// registering 3 shape classes
	// notice, here we know of them through the header
	GManager managerAV("exampleAV", 1);    // no verbosity
	managerAV.RegisterObjectFactory<Triangle>("triangle");
	managerAV.RegisterObjectFactory<Box>("box1");
	managerAV.RegisterObjectFactory<Box>("box2");

	// putting A factory in map
	// notice we're putting the base class in the map so we can call its virtual methods
	map<string, Shape*> fffv;
	fffv["triangle"] = managerAV.CreateObject<Shape>("triangle");
	fffv["box1"]     = managerAV.CreateObject<Shape>("box1");
	fffv["box2"]     = managerAV.CreateObject<Shape>("box2");

	// aShape is same pointer as map element
	Shape* aShape = fffv["triangle"];

	// calling base and derived methods
	aShape->Area();
	fffv["triangle"]->Area();
	fffv["box1"]->Area();

	managerAV.clearDLMap();


	// A manages Shapes
	// registering 3 shape classes
	// notice, here we know of them through the header
	GManager managerA("exampleA");    // no verbosity
	managerA.RegisterObjectFactory<Triangle>("triangle");
	managerA.RegisterObjectFactory<Box>("box1");
	managerA.RegisterObjectFactory<Box>("box2");

	// putting A factory in map
	map<string, Shape*> fff;
	fff["triangle"] = managerA.CreateObject<Shape>("triangle");
	fff["box1"]     = managerA.CreateObject<Shape>("box1");
	fff["box2"]     = managerA.CreateObject<Shape>("box2");

	// aShape is same pointer as map element
	Shape* avShape = fff["triangle"];

	// calling base and derived methods
	avShape->Area();
	fff["triangle"]->Area();
	fff["box1"]->Area();

	managerA.clearDLMap();




	// B manages Cars. Notice, we do not need the derived class headers here!
	// PRAGMA: These two names must match in registerDL and in  LoadAndRegisterObjectFromLibrary:
	// tesla
	// that's ok but need to spit error if that doesn't happen
	GManager managerB("exampleB", 1); // no verbosity of 1
	
	map<string, Car*> ggg;
	ggg["tesla"] = managerB.LoadAndRegisterObjectFromLibrary<Car>("teslaFactory");
	ggg["ford"]  = managerB.LoadAndRegisterObjectFromLibrary<Car>("fordFactory");
	Car* aCar = ggg["ford"];


	// calling base and derived method
	ggg["tesla"]->go();
	ggg["ford"]->go();

	cout << " Shape pointers: " << fff["triangle"] << " " << aShape << endl;
	cout << " Car pointers: " << ggg["ford"] << " " << aCar << endl;

	cout << "generalCarVar from factory map: " << ggg["tesla"]->generalCarVar << endl;

	// why this need to be cleared here, and not after the factories are
	// put in the map?
	managerB.clearDLMap();

}
