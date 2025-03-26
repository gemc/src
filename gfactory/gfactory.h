#ifndef  GFACTORY_H
#define  GFACTORY_H 1

// c++
#include <map>
#include <string>

// dynamic loading
#include "gdl.h"

/*
 * @class GFactoryBase
 * @brief Base class. Use polymorphism to run Create on the
 * derived classes
 */
class GFactoryBase {
public:
    /**
     @brief Pure virtual method. Derived classes use this to instantiate themselves
     */
    virtual void *Create() = 0;
};

/**
 * @brief Derived from GFactoryBase, implements Create to
 * instantiate the derived classes
 */
template<class T>
class GFactory : public GFactoryBase {
    /**
     @fn Create
     @brief Instantiate new client class
     */
    virtual void *Create() override {
        return new T();
    }
};

/**
 * @brief Instantiates derived classes either statically or dynamically
 */
class GManager {
private:

    // the map of GFactoryBase is kept on GManager
    std::map<std::string, GFactoryBase *> factoryMap;

    // the reason to keep this map is to keep the (DynamicLib?) pointers in memory
    // for some reason declaring a dynamic_lib local variable in LoadAndRegisterObjectFromLibrary
    // scope does not work
    std::map<std::string, DynamicLib *> dlMap;

    std::string gname; // manager name

    /**
     * @fn registerDL
     * @param name base name of the dynamic library to be registered
     *
     * The full filename is OS dependent
     */
    void registerDL(std::string name) {
        // PRAGMA TODO: make it OS independent?
        dlMap[name] = new DynamicLib(gopts, name + ".gplugin");
		log.debug(NORMAL, "Loading DL ", name);
	}

	GOptions *gopts;
	GLogger log;

public:
    /**
     @param v verbosity.
     - 0: no not print any log
     - 1: print gmanager registering and instantiating classes
     */
    GManager(GOptions* g, std::string name) :gname(name),  gopts(g), log(g, "plugins") {
		log.debug(CONSTRUCTOR, "Instantiating ", gname);
	}

public:
    /**
     * @param name name under which the factory is registered
     *
     * Before instantiating the wanted class we need to register
     * it (as Derived) in the manager first using RegisterObjectFactory.\n
     * Derived is the derived class, i.e. "Triangle" : "Shape".\n
     * The client knows about the derived class (through header)
     */
    template<class Derived>
    void RegisterObjectFactory(std::string name) {
        factoryMap[name] = new GFactory<Derived>();
		log.debug(NORMAL, "Registering ", name, "into factory Map");

	}

    /**
     * @fn CreateObject
     * @param name name under which the factory is registered
     *
     * After registration we can create the object using CreateObject.\n
     * Notice that Base here is the client base class.\n
     * c++ polymorphism is used to called the client methods derived
     * from the base pure virtual methods.
     */
    template<class Base>
    Base *CreateObject(std::string name) const {

        auto factory = factoryMap.find(name);

        if (factory == factoryMap.end()) {
			log.error(EC__FACTORYNOTFOUND, "couldn't find factory ", name, " in factory Map.");
        }

		log.debug(NORMAL, "Creating instance of ", name, " factory.");
        return static_cast<Base *>(factory->second->Create());
    }


    /**
     * @fn LoadAndRegisterObjectFromLibrary
     * @param name name under which the factory is registered
     *
     * Instantiate client derived class.\n
     * Notice the base class must have the static method instantiate
     */
    template<class T>
    T *LoadAndRegisterObjectFromLibrary(std::string name) {

        registerDL(name);

        // will return nullptr if handle is null
        DynamicLib *dynamicLib = dlMap[name];
        if (dynamicLib != nullptr) {
            dlhandle thisDLHandle = dynamicLib->handle;
            if (thisDLHandle != nullptr) {
                return T::instantiate(thisDLHandle);
            }
        } else {
            // warning message already given if plugin not found
			log.error(EC__DLHANDLENOTFOUND, "plugin ", name, " could not be loaded.");
        }
        return nullptr;
    }

    /**
     * @fn destroyObject
     * @param object class type of the instance to be deleted
     *
     * Delete the instance pointer
     */
    //	template <class T> void destroyObject(T* object) {
    //		delete object;
    //	}

    /**
     * @fn clearDLMap
     *
     * Delete the various dynamic libraries handle
     * This should be done at the end of the program
     */
    void clearDLMap() {
        for (auto &i: dlMap) {
            delete i.second;
        }
    }

};

#endif
