// g4system
#include "g4NativeObjectsFactory.h"

// glibrary
#include "gutilities.h"
using namespace gutilities;

// geant4
#include "G4Box.hh"
#include "G4Sphere.hh"
#include "G4Tubs.hh"
#include "G4CutTubs.hh"
#include "G4Cons.hh"
#include "G4Para.hh"
#include "G4Trap.hh"
#include "G4Trd.hh"
#include "G4Polycone.hh"

G4VSolid* G4NativeSystemFactory::buildSolid(GOptions* gopt, GVolume *s, map<string, G4Volume*> *g4s)
{
	string g4name = s->getG4Name();
	bool verbosity = getVerbosity(gopt, g4name);

	// check dependencies first
	if(!checkSolidDependencies(verbosity, s, g4s)) return nullptr;

	// if the g4volume doesn't exist, create one and add it to the map
	G4Volume *thisG4Volume = nullptr;

	// check if g4s already exists
	if(g4s->find(g4name) != g4s->end()) {
		thisG4Volume = (*g4s)[g4name];
		// if the solid is already built, returning it
		if (thisG4Volume->getSolid() != nullptr) return thisG4Volume->getSolid();
	} else {
		thisG4Volume = new G4Volume();
		// adding volume to the map
		(*g4s)[g4name] = thisG4Volume;
	}

	// geant4 solids definitions:
	// https://geant4-userdoc.web.cern.ch/UsersGuides/ForApplicationDeveloper/html/Detector/Geometry/geomSolids.html

	// check and get parameters
	vector<double> pars = checkAndReturnParameters(s);

	string type = s->getType();

	if(type == "G4Box") {
		thisG4Volume->setSolid(new G4Box(g4name,     // name
													pars[0],    // half length in X
													pars[1],    // half length in Y
													pars[2]     // half length in Z
													), verbosity
									  );

		return thisG4Volume->getSolid();

	} else 	if(type == "G4Tubs") {
		thisG4Volume->setSolid(new G4Tubs(g4name,    // name
													 pars[0],   // Inner radius
													 pars[1],   // Outer radius
													 pars[2],   // Half length in z
													 pars[3],   // Starting phi angle
													 pars[4]    // Delta Phi angle
													 ), verbosity
									  );

		return thisG4Volume->getSolid();

	} else 	if(type == "G4Sphere") {
		thisG4Volume->setSolid(new G4Sphere(g4name,   // name
													 pars[0],    // Inner radius
													 pars[1],    // Outer radius
													 pars[2],    // Starting phi angle
													 pars[3],    // Delta Phi angle
													 pars[4],    // Starting delta angle
													 pars[5]     // Delta delta angle
													 ), verbosity
									  );

		return thisG4Volume->getSolid();

	} else 	if(type == "G4CutTubs") {
		thisG4Volume->setSolid(new G4CutTubs(g4name,    // name
														 pars[0],   // Inner radius
														 pars[1],   // Outer radius
														 pars[2],   // Half length in z
														 pars[3],   // Starting phi angle
														 pars[4],   // Delta Phi angle
														 G4ThreeVector(pars[5], pars[6], pars[7]),    // Outside Normal at -z
														 G4ThreeVector(pars[8], pars[9], pars[10])    // Outside Normal at +z
														 ), verbosity
									  );

		return thisG4Volume->getSolid();

	} else 	if(type == "G4Cons") {
		thisG4Volume->setSolid(new G4Cons(g4name,    // name
													 pars[0],   // Inside radius at -pDz
													 pars[1],   // Outside radius at -pDz
													 pars[2],   // Inside radius at +pDz
													 pars[3],   // Outside radius at +pDz
													 pars[4],   // Half length in z
													 pars[5],   // Starting phi angle
													 pars[6]    // Delta Phi angle
													 ), verbosity
									  );

		return thisG4Volume->getSolid();

	} else 	if(type == "G4Para") {
		thisG4Volume->setSolid(new G4Para(g4name,    // name
													 pars[0],   // Half length in x
													 pars[1],   // Half length in y
													 pars[2],   // Half length in z
													 pars[3],   // Angle formed by the y axis and by the plane joining the centre of the faces parallel to the z-x plane at -dy and +dy
													 pars[4],   // Polar angle of the line joining the centres of the faces at -dz and +dz in z
													 pars[5]    // Azimuthal angle of the line joining the centres of the faces at -dz and +dz in z
													 ), verbosity
									  );

		return thisG4Volume->getSolid();

	} else 	if(type == "G4Trd") {
		thisG4Volume->setSolid(new G4Trd(g4name,    // name
													pars[0],   // Half-length along x at the surface positioned at -dz
													pars[1],   // Half-length along x at the surface positioned at +dz
													pars[2],   // Half-length along y at the surface positioned at -dz
													pars[3],   // Half-length along y at the surface positioned at +dz
													pars[4]    // Half-length along z axis
													), verbosity
									  );
		return thisG4Volume->getSolid();
	} else 	if(type == "G4Trap") {

		// G4Trap has three main constructors:
		// - for a Right Angular Wedge (4 parameters)
		// - for a general trapezoid (11 parameters)
		// - from eight points (24 parameters for 8 G4ThreeVectors)


		// Right Angular Wedge (4 parameters)
		if ( pars.size() == 4 ) {
			thisG4Volume->setSolid(new G4Trap(g4name,    // name
														 pars[0],   // Length along Z
														 pars[1],   // Length along Y
														 pars[2],   // Length along X wider side
														 pars[3]    // Length along X at the narrower side (plTX<=pX)
														 ), verbosity
										  );
			// general trapezoid (11 parameters)
		} else if ( pars.size() == 11) {
			thisG4Volume->setSolid(new G4Trap(g4name,     // name
														 pars[0],    // Half Z length - distance from the origin to the bases
														 pars[1],    // Polar angle of the line joining the centres of the bases at -/+pDz
														 pars[2],    //  Azimuthal angle of the line joining the centre of the base at -pDz to the centre of the base at +pDz
														 pars[3],    // Half Y length of the base at -pDz
														 pars[4],    // Half Y length of the base at +pDz
														 pars[5],    // Half X length at smaller Y of the base at -pDz
														 pars[6],    // Half X length at bigger Y of the base at -pDz
														 pars[7],    // Half X length at smaller Y of the base at +pDz
														 pars[8],    // Half X length at bigger y of the base at +pDz
														 pars[9],    // Angle between the Y-axis and the centre line of the base at -pDz (lower endcap)
														 pars[10]    // Angle between the Y-axis and the centre line of the base at +pDz (upper endcap)
														 ), verbosity
										  );

			// from eight points (8 parameters)
		} else if ( pars.size() == 24) {
			G4ThreeVector pt[8];

			pt[0] = G4ThreeVector(pars[0],  pars[1],  pars[2]);
			pt[1] = G4ThreeVector(pars[3],  pars[4],  pars[5]);
			pt[2] = G4ThreeVector(pars[6],  pars[7],  pars[8]);
			pt[3] = G4ThreeVector(pars[9],  pars[10], pars[11]);
			pt[4] = G4ThreeVector(pars[12], pars[13], pars[14]);
			pt[5] = G4ThreeVector(pars[15], pars[16], pars[17]);
			pt[6] = G4ThreeVector(pars[18], pars[19], pars[20]);
			pt[7] = G4ThreeVector(pars[21], pars[22], pars[23]);

			thisG4Volume->setSolid(new G4Trap(g4name,  // name
														 pt       // Coordinates of the vertices
														 ), verbosity
										  );

		} else {
			G4cerr << " " << g4name << " solid " << type << " constructor must have 4, 11 or 24 parameters, \
         see  https://geant4-userdoc.web.cern.ch/UsersGuides/ForApplicationDeveloper/html/Detector/Geometry/geomSolids.html " << G4endl;
		}

		return thisG4Volume->getSolid();

	} else if(type == "G4Polycone") {

		double phistart = pars[0];
		double phitotal = pars[1];
		int zplanes    = (int) pars[2];

		double* zPlane = new double[zplanes];
		double* rInner = new double[zplanes];
		double* rOuter = new double[zplanes];

		for(int zpl=0; zpl<zplanes; zpl++) {
			zPlane[zpl] = pars[3 + 0*zplanes + zpl] ;
			rInner[zpl] = pars[3 + 1*zplanes + zpl] ;
			rOuter[zpl] = pars[3 + 2*zplanes + zpl] ;
		}

		thisG4Volume->setSolid(new G4Polycone(g4name,         // name
														  phistart,       // Initial Phi starting angle
														  phitotal,       // Total Phi angle
														  zplanes,        // Number of z planes
														  zPlane,         // z coordinate of corners
														  rInner,         // Tangent distance to inner surface
														  rOuter          // Tangent distance to outer surface, verbosity
														  ), verbosity
									  );

		return thisG4Volume->getSolid();
	} else {
		G4cerr << " " << g4name << " solid " << type << " unknown! \
		See https://geant4-userdoc.web.cern.ch/UsersGuides/ForApplicationDeveloper/html/Detector/Geometry/geomSolids.html \
      for the list of supported constructor" << G4endl;
	}

	// if we are at this point the solid is not built
	if(verbosity) {
		G4cout << " " << g4name << " solid is not built." << G4endl;
	}

	return nullptr;
}





