#ifndef  GSYSTEMOPTIONS_H
#define  GSYSTEMOPTIONS_H 1

// glibrary
#include "goptions.h"

namespace gsystem {

// System
// ------

struct JSystem {
	string system;
	string factory;
	string variation;
    string annotations;
	int runno;
    string sqlite_file;
};

void from_json(const json& j, JSystem& det);

// method to return a vector of JSystem from a structured option
vector<JSystem> getSystems(GOptions *gopts);


// Modifier
// --------

struct JModifier {
	string volume;    // volume name
	string shift;
	string tilt;
	bool isPresent;  // true by default
};

void from_json(const json& j, JModifier& mods);

// method to return a vector of modifiers from a structured option
vector<JModifier> getModifiers(GOptions *gopts);


// returns array of options definitions
vector<GOption> defineOptions();
}


#endif
