// gphysics
#include "gphysicsConventions.h"
#include "gphysicsOptions.h"

// geant4 version
#include "G4Version.hh"

using namespace std;

// namespace to define options
namespace gphysics {

// returns array of options definitions
vector<GOption> defineOptions() {
	
	vector<GOption> goptions;
	
	json jsonGPhysVerbosity = {
		{GNAME, GPHYSVERBOSITY},
        {GDESC, "Verbosity for gphysics. " + string(GVERBOSITY_DESCRIPTION)},
		{GDFLT, 0}
	};
	goptions.push_back(GOption(jsonGPhysVerbosity));
	
	string physicsListHelp = "Geant4 Version " + string(G4VERSION_TAG) ;
	
	physicsListHelp += " Physics List: it contains a Geant4 physics module, optional e.m. replacement, and optional physics constructors\n";
	// below is the output of ./formatPhysList.py -gemc
	physicsListHelp += string(HELPFILLSPACE) + "The available geant4 modules are:\n\n";
	physicsListHelp += string(HELPFILLSPACE) + " - FTFP_BERT\n";
	physicsListHelp += string(HELPFILLSPACE) + " - FTFP_BERT_ATL\n";
	physicsListHelp += string(HELPFILLSPACE) + " - FTFP_BERT_HP\n";
	physicsListHelp += string(HELPFILLSPACE) + " - FTFP_BERT_TRV\n";
	physicsListHelp += string(HELPFILLSPACE) + " - FTFP_INCLXX\n";
	physicsListHelp += string(HELPFILLSPACE) + " - FTFP_INCLXX_HP\n";
	physicsListHelp += string(HELPFILLSPACE) + " - FTFQGSP_BERT\n";
	physicsListHelp += string(HELPFILLSPACE) + " - FTF_BIC\n";
	physicsListHelp += string(HELPFILLSPACE) + " - G4GenericPhysicsList\n";
	physicsListHelp += string(HELPFILLSPACE) + " - LBE\n";
	physicsListHelp += string(HELPFILLSPACE) + " - NuBeam\n";
	physicsListHelp += string(HELPFILLSPACE) + " - QBBC\n";
	physicsListHelp += string(HELPFILLSPACE) + " - QGSP_BERT\n";
	physicsListHelp += string(HELPFILLSPACE) + " - QGSP_BERT_HP\n";
	physicsListHelp += string(HELPFILLSPACE) + " - QGSP_BIC\n";
	physicsListHelp += string(HELPFILLSPACE) + " - QGSP_BIC_AllHP\n";
	physicsListHelp += string(HELPFILLSPACE) + " - QGSP_BIC_HP\n";
	physicsListHelp += string(HELPFILLSPACE) + " - QGSP_FTFP_BERT\n";
	physicsListHelp += string(HELPFILLSPACE) + " - QGSP_INCLXX\n";
	physicsListHelp += string(HELPFILLSPACE) + " - QGSP_INCLXX_HP\n";
	physicsListHelp += string(HELPFILLSPACE) + " - QGS_BIC\n";
	physicsListHelp += string(HELPFILLSPACE) + " - Shielding\n";
	physicsListHelp += string(HELPFILLSPACE) + " - ShieldingLEND\n";
	physicsListHelp += string(HELPFILLSPACE) + " - ShieldingM\n";
	physicsListHelp += "\n" + string(HELPFILLSPACE) + "The default e.m. physics can be replaced by appending one of these string to the module above:\n\n";
	physicsListHelp += string(HELPFILLSPACE) + " - _EM0 to use G4EmStandardPhysics\n" ;
	physicsListHelp += string(HELPFILLSPACE) + " - _EMV to use G4EmStandardPhysics_option1\n" ;
	physicsListHelp += string(HELPFILLSPACE) + " - _EMX to use G4EmStandardPhysics_option2\n" ;
	physicsListHelp += string(HELPFILLSPACE) + " - _EMY to use G4EmStandardPhysics_option3\n" ;
	physicsListHelp += string(HELPFILLSPACE) + " - _EMZ to use G4EmStandardPhysics_option4\n" ;
	physicsListHelp += string(HELPFILLSPACE) + " - _GS to use G4EmStandardPhysicsGS\n" ;
	physicsListHelp += string(HELPFILLSPACE) + " - _LE to use G4EmLowEPPhysics\n" ;
	physicsListHelp += string(HELPFILLSPACE) + " - _LIV to use G4EmLivermorePhysics\n" ;
	physicsListHelp += string(HELPFILLSPACE) + " - _PEN to use G4EmPenelopePhysics\n" ;
	physicsListHelp += string(HELPFILLSPACE) + " - _SS to use G4EmStandardPhysicsSS\n" ;
	physicsListHelp += string(HELPFILLSPACE) + " - _WVI to use G4EmStandardPhysicsWVI\n" ;
	physicsListHelp += "\n" + string(HELPFILLSPACE) + "For example, FTFP_BERT_LIV would replace the default e.m. physics with the Livermode model \n\n";
	physicsListHelp += "\n" + string(HELPFILLSPACE) + "Additional physics can be loaded by adding its constructor name to the list using the "+" sign\n";
	physicsListHelp += "\n" + string(HELPFILLSPACE) + "For example: FTFP_BERT + G4OpticalPhysics. The available constructors are: \n\n";
	physicsListHelp += string(HELPFILLSPACE) + " - G4ChargeExchangePhysics\n";
	physicsListHelp += string(HELPFILLSPACE) + " - G4DecayPhysics\n";
	physicsListHelp += string(HELPFILLSPACE) + " - G4EmDNAChemistry\n";
	physicsListHelp += string(HELPFILLSPACE) + " - G4EmDNAChemistry_option1\n";
	physicsListHelp += string(HELPFILLSPACE) + " - G4EmDNAChemistry_option2\n";
	physicsListHelp += string(HELPFILLSPACE) + " - G4EmDNAChemistry_option3\n";
	physicsListHelp += string(HELPFILLSPACE) + " - G4EmDNAPhysics\n";
	physicsListHelp += string(HELPFILLSPACE) + " - G4EmDNAPhysics_option1\n";
	physicsListHelp += string(HELPFILLSPACE) + " - G4EmDNAPhysics_option2\n";
	physicsListHelp += string(HELPFILLSPACE) + " - G4EmDNAPhysics_option3\n";
	physicsListHelp += string(HELPFILLSPACE) + " - G4EmDNAPhysics_option4\n";
	physicsListHelp += string(HELPFILLSPACE) + " - G4EmDNAPhysics_option5\n";
	physicsListHelp += string(HELPFILLSPACE) + " - G4EmDNAPhysics_option6\n";
	physicsListHelp += string(HELPFILLSPACE) + " - G4EmDNAPhysics_option7\n";
	physicsListHelp += string(HELPFILLSPACE) + " - G4EmDNAPhysics_option8\n";
	physicsListHelp += string(HELPFILLSPACE) + " - G4EmDNAPhysics_stationary\n";
	physicsListHelp += string(HELPFILLSPACE) + " - G4EmDNAPhysics_stationary_option2\n";
	physicsListHelp += string(HELPFILLSPACE) + " - G4EmDNAPhysics_stationary_option4\n";
	physicsListHelp += string(HELPFILLSPACE) + " - G4EmDNAPhysics_stationary_option6\n";
	physicsListHelp += string(HELPFILLSPACE) + " - G4EmExtraPhysics\n";
	physicsListHelp += string(HELPFILLSPACE) + " - G4EmLivermorePhysics\n";
	physicsListHelp += string(HELPFILLSPACE) + " - G4EmLivermorePolarizedPhysics\n";
	physicsListHelp += string(HELPFILLSPACE) + " - G4EmLowEPPhysics\n";
	physicsListHelp += string(HELPFILLSPACE) + " - G4EmPenelopePhysics\n";
	physicsListHelp += string(HELPFILLSPACE) + " - G4EmStandardPhysics\n";
	physicsListHelp += string(HELPFILLSPACE) + " - G4EmStandardPhysicsGS\n";
	physicsListHelp += string(HELPFILLSPACE) + " - G4EmStandardPhysicsSS\n";
	physicsListHelp += string(HELPFILLSPACE) + " - G4EmStandardPhysicsWVI\n";
	physicsListHelp += string(HELPFILLSPACE) + " - G4EmStandardPhysics_option1\n";
	physicsListHelp += string(HELPFILLSPACE) + " - G4EmStandardPhysics_option2\n";
	physicsListHelp += string(HELPFILLSPACE) + " - G4EmStandardPhysics_option3\n";
	physicsListHelp += string(HELPFILLSPACE) + " - G4EmStandardPhysics_option4\n";
	physicsListHelp += string(HELPFILLSPACE) + " - G4FastSimulationPhysics\n";
	physicsListHelp += string(HELPFILLSPACE) + " - G4GenericBiasingPhysics\n";
	physicsListHelp += string(HELPFILLSPACE) + " - G4HadronDElasticPhysics\n";
	physicsListHelp += string(HELPFILLSPACE) + " - G4HadronElasticPhysics\n";
	physicsListHelp += string(HELPFILLSPACE) + " - G4HadronElasticPhysicsHP\n";
	physicsListHelp += string(HELPFILLSPACE) + " - G4HadronElasticPhysicsLEND\n";
	physicsListHelp += string(HELPFILLSPACE) + " - G4HadronElasticPhysicsPHP\n";
	physicsListHelp += string(HELPFILLSPACE) + " - G4HadronElasticPhysicsXS\n";
	physicsListHelp += string(HELPFILLSPACE) + " - G4HadronHElasticPhysics\n";
	physicsListHelp += string(HELPFILLSPACE) + " - G4HadronInelasticQBBC\n";
	physicsListHelp += string(HELPFILLSPACE) + " - G4HadronPhysicsFTFP_BERT\n";
	physicsListHelp += string(HELPFILLSPACE) + " - G4HadronPhysicsFTFP_BERT_ATL\n";
	physicsListHelp += string(HELPFILLSPACE) + " - G4HadronPhysicsFTFP_BERT_HP\n";
	physicsListHelp += string(HELPFILLSPACE) + " - G4HadronPhysicsFTFP_BERT_TRV\n";
	physicsListHelp += string(HELPFILLSPACE) + " - G4HadronPhysicsFTFQGSP_BERT\n";
	physicsListHelp += string(HELPFILLSPACE) + " - G4HadronPhysicsFTF_BIC\n";
	physicsListHelp += string(HELPFILLSPACE) + " - G4HadronPhysicsINCLXX\n";
	physicsListHelp += string(HELPFILLSPACE) + " - G4HadronPhysicsNuBeam\n";
	physicsListHelp += string(HELPFILLSPACE) + " - G4HadronPhysicsQGSP_BERT\n";
	physicsListHelp += string(HELPFILLSPACE) + " - G4HadronPhysicsQGSP_BERT_HP\n";
	physicsListHelp += string(HELPFILLSPACE) + " - G4HadronPhysicsQGSP_BIC\n";
	physicsListHelp += string(HELPFILLSPACE) + " - G4HadronPhysicsQGSP_BIC_AllHP\n";
	physicsListHelp += string(HELPFILLSPACE) + " - G4HadronPhysicsQGSP_BIC_HP\n";
	physicsListHelp += string(HELPFILLSPACE) + " - G4HadronPhysicsQGSP_FTFP_BERT\n";
	physicsListHelp += string(HELPFILLSPACE) + " - G4HadronPhysicsQGS_BIC\n";
	physicsListHelp += string(HELPFILLSPACE) + " - G4HadronPhysicsShielding\n";
	physicsListHelp += string(HELPFILLSPACE) + " - G4HadronPhysicsShieldingLEND\n";
	physicsListHelp += string(HELPFILLSPACE) + " - G4ImportanceBiasing\n";
	physicsListHelp += string(HELPFILLSPACE) + " - G4IonBinaryCascadePhysics\n";
	physicsListHelp += string(HELPFILLSPACE) + " - G4IonElasticPhysics\n";
	physicsListHelp += string(HELPFILLSPACE) + " - G4IonINCLXXPhysics\n";
	physicsListHelp += string(HELPFILLSPACE) + " - G4IonPhysics\n";
	physicsListHelp += string(HELPFILLSPACE) + " - G4IonPhysicsPHP\n";
	physicsListHelp += string(HELPFILLSPACE) + " - G4IonPhysicsXS\n";
	physicsListHelp += string(HELPFILLSPACE) + " - G4IonQMDPhysics\n";
	physicsListHelp += string(HELPFILLSPACE) + " - G4MuonicAtomDecayPhysics\n";
	physicsListHelp += string(HELPFILLSPACE) + " - G4NeutronCrossSectionXS\n";
	physicsListHelp += string(HELPFILLSPACE) + " - G4NeutronTrackingCut\n";
	physicsListHelp += string(HELPFILLSPACE) + " - G4OpticalPhysics\n";
	physicsListHelp += string(HELPFILLSPACE) + " - G4ParallelWorldPhysics\n";
	physicsListHelp += string(HELPFILLSPACE) + " - G4RadioactiveDecayPhysics\n";
	physicsListHelp += string(HELPFILLSPACE) + " - G4SpinDecayPhysics\n";
	physicsListHelp += string(HELPFILLSPACE) + " - G4StepLimiterPhysics\n";
	physicsListHelp += string(HELPFILLSPACE) + " - G4StoppingPhysics\n";
	physicsListHelp += string(HELPFILLSPACE) + " - G4StoppingPhysicsFritiofWithBinaryCascade\n";
	physicsListHelp += string(HELPFILLSPACE) + " - G4UnknownDecayPhysics\n";
	physicsListHelp += string(HELPFILLSPACE) + " - G4WeightWindowBiasing\n";

	// end of parsing python script output
	json jsonGPhysList = {
		{GNAME, "physicsList"},
		{GDESC, physicsListHelp},
		{GDFLT, DEFAULTPHYSICSLIST}
	};
	goptions.push_back(GOption(jsonGPhysList));
	
	// log available physics constructors
	goptions.push_back(GOption("showAvailablePhysicsX",  "Log Geant4 Physics Modules that can be used with the \"physicsList\" option and exit"));
	goptions.push_back(GOption("showAvailablePhysics",   "Log Geant4 Physics Modules that can be used with the \"physicsList\" option"));

	return goptions;
}


}
