#ifndef  GWORLD_H
#define  GWORLD_H 1

// gsystem
#include "gsystem.h"
#include "gmodifier.h"
#include "gsystemOptions.h"

// a world is a collection of GSystem, their
class GWorld {

public:
    // constructor from a jcard / command lines:
    //
    // - load systems and modifiers map
    // - load factories
    // - run factory load system for each item in gsystemsMap
    // - apply modifiers
    GWorld(GOptions *gopts);

    ~GWorld() {
        delete gsystemsMap;
    }

private:
    map<string, GSystem *> *gsystemsMap;    // key is system name
    map<string, GModifier *> gmodifiersMap;  // key is volume name

    // search for a volume among systems in gsystemsMap
    // cannot return const because this is used in the constructor to apply shifts
    GVolume *searchForVolume(string volumeName, string purpose) const;
    int verbosity;

public:
    map<string, GSystem *> *getSystemsMap() const { return gsystemsMap; }

    // sensitive detector names, needed to load the plugins at run time
    vector <string> getSensitiveDetectorsList();

};


#endif
