#include "gfactory.h"
#include "gfactory_options.h"
#include "ShapeFactory.h"
#include "Car.h"

// c++
// See header for API docs.
using std::map;
using std::string;

// This example demonstrates both workflows supported by GManager:
//
// 1) Static registration:
//    - We compile/link against the derived classes (Triangle, Box),
//      register them in the manager, and instantiate them by key.
//
// 2) Dynamic loading:
//    - We compile/link only against the base class (Car).
//      Derived classes live in shared libraries and are instantiated by symbol lookup.

int main(int argc, char* argv[]) {
	// Build the options object from argc/argv and the module-defined option set.
	auto gopts = std::make_shared<GOptions>(argc, argv, gfactory::defineOptions());

	// Extra plugins log in here, so we have 3 total (2 additional managers).
	// This is ok in the example; in a practical application we would typically have one manager.
	auto log = std::make_shared<GLogger>(gopts, SFUNCTION_NAME, PLUGIN_LOGGER);

	// -------------------------
	// A) Static factory example
	// -------------------------
	GManager managerAV(gopts);

	// Register the compile-time-known derived classes under keys.
	managerAV.RegisterObjectFactory<Triangle>("triangle", gopts);
	managerAV.RegisterObjectFactory<Box>("box1", gopts);
	managerAV.RegisterObjectFactory<Box>("box2", gopts);

	// Store instances as base pointers to exercise virtual dispatch.
	map<string, Shape*> fffv;
	fffv["triangle"] = managerAV.CreateObject<Shape>("triangle");
	fffv["box1"]     = managerAV.CreateObject<Shape>("box1");
	fffv["box2"]     = managerAV.CreateObject<Shape>("box2");

	// aShape is same pointer as map element.
	Shape* aShape = fffv["triangle"];

	// Calling base and derived methods (virtual dispatch via base pointer).
	aShape->Area();
	fffv["triangle"]->Area();
	fffv["box1"]->Area();

	log->info(0, " Shape pointers from map: ", fffv["triangle"], ", from direct pointer:", aShape);


	// --------------------------
	// B) Dynamic loading example
	// --------------------------
	// Notice: we do not need the derived class headers here.
	//
	// Convention: These names must match the library naming rule used by the loader
	// (registerDL uses "<name>.gplugin") and the name passed to the load call.
	GManager managerB(gopts);

	map<string, std::shared_ptr<Car>> ggg;

	// Load plugins and create instances. The returned shared_ptr keeps the library loaded
	// for the object's lifetime.
	ggg["tesla"] = managerB.LoadAndRegisterObjectFromLibrary<Car>("test_dyn_factory1", gopts);
	ggg["ford"]  = managerB.LoadAndRegisterObjectFromLibrary<Car>("test_dyn_factory2", gopts);
	auto aCar    = ggg["ford"];

	// Calling base and derived method through base-class interface.
	ggg["tesla"]->go();
	ggg["ford"]->go();

	log->info(0, " Car pointers from map: ", ggg["ford"], ", from direct pointer:", aCar);
	log->info(0, " run generalCarVar method from factory map: ", ggg["tesla"]->generalCarVar);

	return EXIT_SUCCESS;
}
