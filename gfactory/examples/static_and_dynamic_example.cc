#include "gfactory.h"
#include "gfactory_options.h"
#include "ShapeFactory.h"
#include "Car.h"

// c++
using std::map;
using std::string;

// managerA: loads known classes Triangle and Box from the base Shape, in ShapeFactory.h
// managerB: loads DLL that instantiate derived factories - we only know of the base class, in this case "Car"

int main(int argc, char *argv[]) {

	auto gopts = new GOptions(argc, argv, gfactory::defineOptions());
	auto log = std::make_shared<GLogger>(gopts, PLUGIN_LOGGER, "plugin_example");

	GManager managerAV(log, "GManager Static");
	managerAV.RegisterObjectFactory<Triangle>("triangle");
	managerAV.RegisterObjectFactory<Box>("box1");
	managerAV.RegisterObjectFactory<Box>("box2");

	// putting A factory in map
	// notice we're putting the base class in the map so we can call its virtual methods
	map<string, Shape *> fffv;
	fffv["triangle"] = managerAV.CreateObject<Shape>("triangle");
	fffv["box1"] = managerAV.CreateObject<Shape>("box1");
	fffv["box2"] = managerAV.CreateObject<Shape>("box2");

	// aShape is same pointer as map element
	Shape *aShape = fffv["triangle"];

	// calling base and derived methods
	aShape->Area();
	fffv["triangle"]->Area();
	fffv["box1"]->Area();

	log->info(0, " Shape pointers from map: ", fffv["triangle"], ", from direct pointer:", aShape);

	// once we're done with it
	managerAV.clearDLMap();


	// B manages Cars. Notice, we do not need the derived class headers here!
	// PRAGMA: These two names must match in registerDL and in LoadAndRegisterObjectFromLibrary:
	// that's ok but need to spit error if that doesn't happen
	GManager managerB(log, "GManager Dynamic");

	map<string, std::shared_ptr<Car>> ggg;
	ggg["tesla"] = managerB.LoadAndRegisterObjectFromLibrary<Car>("test_dyn_factory1", gopts);
	ggg["ford"] = managerB.LoadAndRegisterObjectFromLibrary<Car>("test_dyn_factory2", gopts);
	auto aCar = ggg["ford"];


	// calling base and derived method
	ggg["tesla"]->go();
	ggg["ford"]->go();

	log->info(0, " Car pointers from map: ", ggg["ford"], ", from direct pointer:", aCar);
	log->info(0, " run generalCarVar method from factory map: ", ggg["tesla"]->generalCarVar);

	// clearing the map - this should be done in classes destructors
	managerB.clearDLMap();
}
