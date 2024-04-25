// gsystem
#include "gmaterial.h"

// c++
#include <iostream>
#include <sstream>
using namespace std;


GMaterial::GMaterial(string s, vector<string> pars) : system(s)
{
	if( pars.size() != GMATERIALNUMBEROFPARS) {
		cerr << FATALERRORL << "incorrect number of material parameters (" << pars.size() << ") for " << pars[0] ;
		for ( auto& parameter: pars  ) {
			cerr << " par " << parameter << endl;
		}
		cerr << " It should be " << GMATERIALNUMBEROFPARS << endl;
		gexit(EC__GWRONGNUMBEROFPARS);
	} else {
		// size is already checked in addVolume, the only interface to volume
		int i=0;
		
		name         = removeAllSpacesFromString(pars[i++]);
		density      = stod(removeAllSpacesFromString(pars[i++]));

		// sets components and amounts
		setComponentsFromString(pars[i++]);

        // description
        description  = pars[i++];

        // optical properties
		getMaterialPropertyFromString(pars[i++], "photonEnergy");
		getMaterialPropertyFromString(pars[i++], "indexOfRefraction");
		getMaterialPropertyFromString(pars[i++], "absorptionLength");
		getMaterialPropertyFromString(pars[i++], "reflectivity");
		getMaterialPropertyFromString(pars[i++], "efficiency");

        // scintillation properties
		getMaterialPropertyFromString(pars[i++], "fastcomponent");
		getMaterialPropertyFromString(pars[i++], "slowcomponent");

		scintillationyield = stod(removeLeadingAndTrailingSpacesFromString(pars[i++]));
		resolutionscale    = stod(removeLeadingAndTrailingSpacesFromString(pars[i++]));
		fasttimeconstant   = stod(removeLeadingAndTrailingSpacesFromString(pars[i++]));
		slowtimeconstant   = stod(removeLeadingAndTrailingSpacesFromString(pars[i++]));
		yieldratio         = stod(removeLeadingAndTrailingSpacesFromString(pars[i++]));
		birkConstant       = stod(removeLeadingAndTrailingSpacesFromString(pars[i++]));

        // other optical processes
		getMaterialPropertyFromString(pars[i++], "rayleigh");

	}
}


ostream &operator<<(ostream &stream, GMaterial gMat)
{
	stream  << endl;
	stream << "   - Material: "    << gMat.name     << "  in system  " <<  gMat.system << ": " <<  endl;
	stream << "     Density:          "    << gMat.density      << endl;
	if ( gMat.components.size() > 0 ) {
		stream << "     Composition:          "    << endl;
		for ( unsigned m=0; m<gMat.components.size(); m++ ) {
			string quantity = gMat.amounts[m] > 1 ? " atoms " : " fractional mass";
			stream << "       ・ " << gMat.components[m] << quantity << " " << gMat.amounts[m] << endl;
		}
	}
    stream << "     Description: " << gMat.description << endl;
	stream  << endl;

	return stream;
}


// sets components and amounts
void GMaterial::setComponentsFromString(string composition) {

	vector<string> allComponents = getStringVectorFromString(composition);

	for( unsigned e=0; e < allComponents.size()/2; e++ ) {
		components.push_back(allComponents[e*2]);
		amounts.push_back(stod(allComponents[e*2+1]));
	}

}


// load property from DB entry based on its name
void GMaterial::getMaterialPropertyFromString(string parameter, string propertyName) {

	// nothing to do if the parameter is not assigned
	if ( removeLeadingAndTrailingSpacesFromString(parameter) == GMATERIALNOTASSIGNEDPROPERTYSTRING ) {
		return;
	}

	stringstream parameterComponents(parameter);

	while(!parameterComponents.eof()) {
		string component;
		parameterComponents >> component ;

		// removing whitespaces
		string trimmedComponent = removeLeadingAndTrailingSpacesFromString(component);

		if( propertyName == "photonEnergy" ) {
			photonEnergy.push_back(getG4Number(trimmedComponent));
		} else if( propertyName == "indexOfRefraction" ) {
			indexOfRefraction.push_back(getG4Number(trimmedComponent));
		} else if(propertyName == "absorptionLength") {
			absorptionLength.push_back(getG4Number(trimmedComponent));
		} else if(propertyName == "reflectivity") {
			reflectivity.push_back(getG4Number(trimmedComponent));
		} else if(propertyName == "efficiency") {
			efficiency.push_back(getG4Number(trimmedComponent));
		} else if(propertyName == "fastcomponent") {
			fastcomponent.push_back(getG4Number(trimmedComponent));
		} else if(propertyName == "slowcomponent") {
			slowcomponent.push_back(getG4Number(trimmedComponent));
		} else if(propertyName == "scintillationyield") {
			scintillationyield = getG4Number(trimmedComponent);
		} else if(propertyName == "resolutionscale") {
			resolutionscale = getG4Number(trimmedComponent);
		} else if(propertyName == "fasttimeconstant") {
			fasttimeconstant = getG4Number(trimmedComponent);
		} else if(propertyName == "slowtimeconstant") {
			slowtimeconstant = getG4Number(trimmedComponent);
		} else if(propertyName == "yieldratio") {
			yieldratio = getG4Number(trimmedComponent);
		} else if(propertyName == "birkConstant") {
			birkConstant = getG4Number(trimmedComponent);
		} else if(propertyName == "rayleigh") {
			rayleigh.push_back(getG4Number(trimmedComponent));
		}


		if(propertyName == "rayleigh") {
			// rayleigh is the last quantity to be loaded
			// now we can check the vector sizes for comparison
			// if they do match, behaviour is unknown, we need to exit
			unsigned long photonEnergyVectorSize = photonEnergy.size();

			if ( indexOfRefraction.size() > 0 &&  indexOfRefraction.size() != photonEnergyVectorSize ) {
				cerr << FATALERRORL << "indexOfRefraction size " << indexOfRefraction.size() << " mismatch: photonEnergy has size " << photonEnergyVectorSize << endl;
				gexit(EC__GMATERIALOPTICALPROPERTYMISMATCH);
			}
			if ( absorptionLength.size() > 0 &&  absorptionLength.size() != photonEnergyVectorSize ) {
				cerr << FATALERRORL << "absorptionLength size " << absorptionLength.size()   << " mismatch: photonEnergy has size " << photonEnergyVectorSize << endl;
				gexit(EC__GMATERIALOPTICALPROPERTYMISMATCH);
			}
			if ( reflectivity.size() > 0 &&  reflectivity.size() != photonEnergyVectorSize ) {
				cerr << FATALERRORL << "reflectivity size " << reflectivity.size()           << " mismatch: photonEnergy has size " << photonEnergyVectorSize << endl;
				gexit(EC__GMATERIALOPTICALPROPERTYMISMATCH);
			}
			if ( efficiency.size() > 0 &&  efficiency.size() != photonEnergyVectorSize ) {
				cerr << FATALERRORL << "efficiency size " << efficiency.size()               << " mismatch: photonEnergy has size " << photonEnergyVectorSize << endl;
				gexit(EC__GMATERIALOPTICALPROPERTYMISMATCH);
			}
			if ( fastcomponent.size() > 0 &&  fastcomponent.size() != photonEnergyVectorSize ) {
				cerr << FATALERRORL << "fastcomponent size " << fastcomponent.size()         << " mismatch: photonEnergy has size " << photonEnergyVectorSize << endl;
				gexit(EC__GMATERIALOPTICALPROPERTYMISMATCH);
			}
			if ( slowcomponent.size() > 0 &&  slowcomponent.size() != photonEnergyVectorSize ) {
				cerr << FATALERRORL << "slowcomponent size " << slowcomponent.size()         << " mismatch: photonEnergy has size " << photonEnergyVectorSize << endl;
				gexit(EC__GMATERIALOPTICALPROPERTYMISMATCH);
			}
			if ( rayleigh.size() > 0 &&  rayleigh.size() != photonEnergyVectorSize ) {
				cerr << FATALERRORL << "rayleigh size " << rayleigh.size()                   << " mismatch: photonEnergy has size " << photonEnergyVectorSize << endl;
				gexit(EC__GMATERIALOPTICALPROPERTYMISMATCH);
			}

		}



	}
}
