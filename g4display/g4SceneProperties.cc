// g4display 
#include "g4SceneProperties.h"
#include "g4displayOptions.h"
using namespace g4display;

// glibrary
#include "gutilities.h"
using namespace gutilities;

// c++
using namespace std;

// geant4
#include "G4UImanager.hh"


G4SceneProperties::G4SceneProperties(GOptions* gopts)
{
	// PRAGMA TODO: add resolution option, hits options, here?
	// PRAGMA TOOO: if run/beamOn is given (e.g. command line options) then the label needs to be updated!

	// notice gui and verbosity are not set in g4display but in main()
	bool gui   = gopts->getSwitch("gui");
	bool dawn  = gopts->getSwitch("dawn");
	int verbosity = gopts->getInt("g4displayv");

	G4UImanager* g4uim = G4UImanager::GetUIpointer();

	if(g4uim) {

		// projecting options onto JView and JCamera structs
		JView jview = getJView(gopts);
		JCamera jcamera = getJCamera(gopts);

		vector<string> commands;

		commands.push_back("/vis/scene/create gemc");


		if (dawn) {
			commands.push_back("/vis/open DAWNFILE");
			commands.push_back("/vis/geometry/set/visibility World 0 false");
			commands.push_back("/vis/viewer/set/style surface");
		}

		if ( gui ) {
			commands.push_back("/vis/open " + jview.viewer + " " + jview.dimension  + jview.position);
		}
		
		// Disable auto refresh and quieten vis messages whilst scene is established:
		commands.push_back("/vis/viewer/set/autoRefresh false");
		commands.push_back("/vis/drawVolume");


		// scene texts
		for ( string c: addSceneTexts(gopts) ) {
			commands.push_back(c);
		}



		double toDegrees  = 180/3.1415;
		double thetaValue = getG4Number(jcamera.theta)*toDegrees;
		double phiValue   = getG4Number(jcamera.phi)*toDegrees;

		commands.push_back("/vis/viewer/set/viewpointThetaPhi " + to_string(thetaValue) + " " + to_string(phiValue));
		commands.push_back("/vis/viewer/set/lineSegmentsPerCircle " + to_string(jview.segsPerCircle));
		commands.push_back("/vis/viewer/set/autoRefresh true");

		if (dawn) {
			commands.push_back("/vis/viewer/flush");
		}

		for(auto &c : commands) {
			if(verbosity > GVERBOSITY_SUMMARY) {
				cout << TPOINTITEM << "Executing UIManager command \"" << c << "\"" << endl;
			}
			g4uim->ApplyCommand(c.c_str());
		}

	} else {
		cout << " No UIManager found. " << endl;
	}

	
	
}
