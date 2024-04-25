/**
 * \mainpage
 * \section overview Overview
 * This frameworks provides static or dynamic loading of methods (factories) from shared libraries.\n
 * The class GManager registers the client factories and provide methods to instantiate them.
 *
 * \section howto Suggested Use
 * After the gManager registration clients can instantiate
 * the derived class and store them in a map<string, Base>
 * for later use.
 *
 * \section Static client factories
 * In this case the client have access to both the base and the derived class headers.
 * To register a "Triangle" class derived from "Shape":
 * <pre>
 * \#include "ShapeFactory.h"
 * \#include "Triangle.h"
 * 
 *  // map that will contain the derived classes instances
 * map<string, Shape*> shapes;
 * GManager manager;
 *
 *  // register class in the manager
 * manager.RegisterObjectFactory<Triangle>("triangle");

 * // retrieve "triangle", instantiate and store new client Triangle class
 * map["triangle"] = manager.CreateObject<Shape>("triangle");
 *
 * // Method() is pure virtual in Shape. Calling the instance Triangle method here
 * map["triangle"]->Method();
 *
 * </pre>
 *
 * \section dynamicUse Dynamic client factories
 * In this case the client have access to only the base class header.
 * The derived classes are compiled in shared libraries.
 * To register a "ford" class derived from "Car":
 * <pre>
 * \#include "Car.h"
 *
 *  // map that will contain the derived classes instances
 * map<string, Car*> cars;
 * GManager manager;
 *
 * // register the dynamic library (shared object file) in the manager
 * manager.registerDL("fordFactory");
 *
 * // retrieve "fordFactory" from the shared object, instantiate and store new client fordFactory class
 * map["ford"] = manager.LoadAndRegisterObjectFromLibrary<Car>("fordFactory");
 *
 * // Method() is pure virtual in Car. Calling the instance fordFactory method here
 * map["ford"]->Method();
 * </pre>
 * \n\n
 * \author \n &copy; Maurizio Ungaro
 * \author e-mail: ungaro@jlab.org\n\n\n
 */
