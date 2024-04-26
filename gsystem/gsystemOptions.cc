// gsystem
#include "gworld.h"

// project goption to a system
namespace gsystem {

// System
// ------

void from_json(const json& j, JSystem& det) {
	j.at("system").get_to(det.system);
	j.at("factory").get_to(det.factory);
	j.at("variation").get_to(det.variation);
    j.at("annotations").get_to(det.annotations);
	j.at("runno").get_to(det.runno);
    j.at("sqlite_file").get_to(det.sqlite_file);
}

// method to return a vector of GDetectors from a structured option
vector<JSystem> getSystems(GOptions *gopts) {
	
	vector<JSystem> systems;
	
	auto jdets = gopts->getStructuredOptionAssignedValues("gsystem");
	
	// looking over each of the vector<json> items
	for ( const auto& jdet: jdets ) {
		systems.push_back(jdet.get<JSystem>());
	}
	
	return systems;
}


// Modifier
// --------

void from_json(const json& j, JModifier& mods) {
	j.at("volume").get_to(mods.volume);
	j.at("shift").get_to(mods.shift);
	j.at("tilt").get_to(mods.tilt);
	j.at("isPresent").get_to(mods.isPresent);
}

// method to return a vector of GDetectors from a structured option
vector<JModifier> getModifiers(GOptions *gopts){
	
	vector<JModifier> gmods;
	
	auto gmodifiers = gopts->getStructuredOptionAssignedValues("gmodifier");
	
	// looking over each of the vector<json> items
	for ( const auto& mod: gmodifiers ) {
		gmods.push_back(mod.get<JModifier>());
	}
	
	return gmods;
}



// returns array of options definitions
vector<GOption> defineOptions() {
	vector<GOption> goptions;
	
	// System
	// ------
	
	// detector option
	// groupable: can use -add
	json jsonSystemNameTag = {
		{GNAME, "system"},
		{GDESC, "system name (mandatory). For TEXT factories, it may include the path to the file"},
		{GDFLT, NODFLT}
	};
	json jsonSystemFactoryTag = {
		{GNAME, "factory"},
		{GDESC, "factory name (mandatory). Possible choices: TEXT, CAD, GDML"},
		{GDFLT, NODFLT}
	};
	json jsonSystemVariationTag = {
		{GNAME, "variation"},
		{GDESC, "geometry variation (optional, default is \"default\")"},
		{GDFLT, "default"}
	};
	
	json jsonSystemRunnoTag = {
		{GNAME, "runno"},
		{GDESC, "runno (optional, default is 1)"},
		{GDFLT, 1}
	};

    json jsonSystemAnnotationsTag = {
        {GNAME, "annotations"},
        {GDESC, "annotations. Examples: \"mats_only\" (optional, default is \"none\")"},
        {GDFLT, "none"}
    };

    json jsonSystemSqliteFileTag = {
        {GNAME, "sqlite_file"},
        {GDESC, "sqlite file name (optional, default is \"none\")"},
        {GDFLT, "none"}
    };
	
	json jsonDetectorOption = {
		jsonSystemNameTag,
		jsonSystemFactoryTag,
		jsonSystemVariationTag,
        jsonSystemAnnotationsTag,
		jsonSystemRunnoTag,
        jsonSystemSqliteFileTag
	};
	
	vector<string> help;
	help.push_back("A system definition includes the geometry location, factory and variation");
	help.push_back("");
	help.push_back("Example: +gsystem={detector: \"experiments/clas12/targets\", factory: \"TEXT\", variation: \"bonus\"}");
	
	// the last argument refers to "cumulative"
	goptions.push_back(GOption("gsystem", "defines a group of detectors", jsonDetectorOption, help, true));
	
	
	// Modifier
	// --------
	// detector option
	// groupable: can use -add
	json jsonModifierNameTag = {
		{GNAME, "volume"},
		{GDESC, "volume name (optional) "},
		{GDFLT, GSYSTEMNOMODIFIER}
	};
	json jsonModifierShiftTag = {
		{GNAME, "shift"},
		{GDESC, "volume shift added to existing position"},
		{GDFLT, GSYSTEMNOMODIFIER}
	};
	json jsonModifierTiltTag = {
		{GNAME, "tilt"},
		{GDESC, "volume tilt added to existing rotation"},
		{GDFLT, GSYSTEMNOMODIFIER}
	};
	
	json jsonModifierPresentTag = {
		{GNAME, "isPresent"},
		{GDESC, "remove volume from world if set to false"},
		{GDFLT, true}
	};
	
	json jsonModifierOption = {
		jsonModifierNameTag,
		jsonModifierShiftTag,
		jsonModifierTiltTag,
		jsonModifierPresentTag
	};
	
	help.clear();
	help.push_back("The volume modifer can shift, tilt, or delete a volume from the gworld");
	help.push_back("");
	help.push_back("Example: +gmodifier={volume: \"targetCell\", tilt: \"0*deg, 0*deg, -10*deg\" }");
	
	// the last argument refers to "cumulative"
	goptions.push_back(GOption("gmodifier", "modify volume existance or placement", jsonModifierOption, help, true));
	
	// gsystem verbosiry
	json jsonVerbosityOption = {
		{GNAME, "gsystemv"},
		{GDESC, "Verbosity for gsystem. " + string(GVERBOSITY_DESCRIPTION)},
		{GDFLT, 0}
	};
	goptions.push_back(GOption(jsonVerbosityOption));
	
	// world volume, to be done in g4volume
	json jsonWorldVolumeOption = {
		{GNAME, "worldVolume"},
		{GDESC, "geant4 definition for the world volume <root>. Default is G4Box, 15*m, 15*m, 15*m, G4_Air"},
		{GDFLT, "G4Box, 15*m, 15*m, 15*m, G4_AIR"}
	};
	
	goptions.push_back(GOption(jsonWorldVolumeOption));
	
	return goptions;
}


}
