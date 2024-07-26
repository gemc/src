// g4system
#include "g4systemOptions.h"
#include "g4systemConventions.h"

// project goption to a system
namespace g4system {

    // returns array of options definitions
    GOptions defineOptions() {

        GOptions goptions;
        string help;

        help = "All information related to the volume will be logged. \n \n";
        help += "Example: -logVolume=ctof\n";
        goptions.defineOption(GVariable("logVolume", goptions::NODFLT, "log all information for volume"), help);

        help = "Material to be used if the switch \"useDefaultMaterial\" is activated and a material for a volume is not defined. \n \n";
        help += "The default is G4_Galactic\n";
        help += "Example: -defaultMaterial=G4_Air\n";
        goptions.defineOption(GVariable("defaultMaterial", "G4_Galactic", "material to be used if not defined"), help);

        goptions.defineSwitch("useDefaultMaterial", "use material defined by \"defaultMaterial\" option if a volume's material is not defined");

        help = "Check for volumes overlaps.\n \n";
        help += "Possibles values are:\n";
        help += " - 0 (default): no check.\n";
        help += " - 1: check for overlaps at physical volume construction.\n";
        help += " - 2: use the geant4 overlap validator with 10,000 points on the surface\n";
        help += " - Any number greater than 100 : use the geant4 overlap validator with this number of points on the surface";
        goptions.defineOption(GVariable("checkOverlaps", 0, "Check for volumes overlaps"), help);

        goptions.defineSwitch("showPredefinedMaterials", "Log GEMC Predefined Materials");
        goptions.defineSwitch("printSystemsMaterials", "Print the materials used in this simulation");

        return goptions;
    }


}
