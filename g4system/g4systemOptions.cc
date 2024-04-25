// g4system
#include "g4systemOptions.h"
#include "g4systemConventions.h"

// project goption to a system
namespace g4system {

// returns array of options definitions
vector<GOption> defineOptions() {
	
	vector<GOption> goptions;
	
	json jsonG4WorldVerbosity = {
		{GNAME, G4SYSTEMVERBOSITY},
        {GDESC, "Verbosity for g4system. " + string(GVERBOSITY_DESCRIPTION)},
        {GDFLT, 1}
	};
	goptions.push_back(GOption(jsonG4WorldVerbosity));
	
	// use this string option to print out informations about this volume at construction time
	json jsonVolumeLogVerbosity = {
		{GNAME, "logVolume"},
		{GDESC, "log all information for volume"},
		{GDFLT, UNINITIALIZEDSTRINGQUANTITY}
	};
	goptions.push_back(GOption(jsonVolumeLogVerbosity));
	
	// default material to use when a material is not found
	json jsonDefaultMaterial = {
		{GNAME, "defaultMaterial"},
		{GDESC, "default material to be used if the switch \"useDefaultMaterial\" is activated"},
		{GDFLT, "G4_Galactic"}
	};
	goptions.push_back(GOption(jsonDefaultMaterial));
	
	// add a "useDefaultMaterial" switch
	goptions.push_back(GOption("useDefaultMaterial", "use material defined by \"defaultMaterial\" option if a volume's material is not defined"));
	
	string overlapsHelp = "Check for volumes overlaps.\n";
	overlapsHelp += string(HELPFILLSPACE) + "Possibles values are:\n";
	overlapsHelp += string(HELPFILLSPACE) + " - 0 (default): no check.\n";
	overlapsHelp += string(HELPFILLSPACE) + " - 1: check for overlaps at physical volume construction.\n";
	overlapsHelp += string(HELPFILLSPACE) + " - 2: use the geant4 overlap validator with 10,000 points on the surface\n";
	overlapsHelp += string(HELPFILLSPACE) + " - Any number greater than 100 : use the geant4 overlap validator with this number of points on the surface";
	json jsonG4CheckOverlaps = {
		{GNAME, "checkOverlaps"},
		{GDESC, overlapsHelp},
		{GDFLT, 0}
	};
	goptions.push_back(GOption(jsonG4CheckOverlaps));
	
	
	// log materials switches
	goptions.push_back(GOption("logG4Materials",        "Log Geant4 Predefined Materials"));
	goptions.push_back(GOption("printSystemsMaterials", "Print the materials used in this simulation"));
	
	return goptions;
}


}
