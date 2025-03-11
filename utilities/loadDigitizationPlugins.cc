// c++
#include <string>

using namespace std;

// gemc
#include "goptions.h"
#include "gfactory.h"
#include "gtouchableConventions.h"
#include "ginternalDigitization.h"
#include "gemcUtilities.h"
#include "gemcConventions.h"

// loads plugins from sensitive map <names, paths>
// exits if failure
void
loadDigitizationPlugins(GOptions *gopt, vector <string> sdetectors, map<string, GDynamicDigitization *> *gDDGlobal) {

	int verbosity = gopt->getVerbosityFor("gsensitivity");

	for (auto &sdname: sdetectors) {

		if (sdname == FLUXNAME) {
			(*gDDGlobal)[sdname] = new GFluxDigitization();
			(*gDDGlobal)[sdname]->defineReadoutSpecs();
		} else if (sdname == COUNTERNAME) {
			(*gDDGlobal)[sdname] = new GParticleCounterDigitization();
			(*gDDGlobal)[sdname]->defineReadoutSpecs();
		} else if (sdname == DOSIMETERNAME) {
			(*gDDGlobal)[sdname] = new GDosimeterDigitization();
			(*gDDGlobal)[sdname]->defineReadoutSpecs();

		} else {

			if (verbosity >= GVERBOSITY_SUMMARY) {
				cout << GEMCLOGMSGITEM << "Loading plugins from file " << sdname << endl;
			}

			GManager sdPluginManager(sdname + " GSensitiveDetector", verbosity);

			if (gDDGlobal->find(sdname) == gDDGlobal->end()) {
				(*gDDGlobal)[sdname] = sdPluginManager.LoadAndRegisterObjectFromLibrary<GDynamicDigitization>(sdname);
				(*gDDGlobal)[sdname]->defineReadoutSpecs();

			}

			// done with sdPluginManager
			//sdPluginManager.clearDLMap();
		}
	}
}
