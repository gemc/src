// guts
#include "gutilities.h"
using namespace gutilities;

// g4system
#include "../g4systemConventions.h"
#include "g4objectsFactory.h"


G4VSolid* G4ObjectsFactory::getSolidFromMap(string vname, map<string, G4Volume*> *g4s) const
{

	if(g4s->find(vname) != g4s->end()) {
		auto thisGVolume = (*g4s)[vname];
		return thisGVolume->getSolid();
	}

	return nullptr;
}

G4LogicalVolume* G4ObjectsFactory::getLogicalFromMap(string vname, map<string, G4Volume*> *g4s) const
{

	if(g4s->find(vname) != g4s->end()) {
		auto thisGVolume = (*g4s)[vname];
		return thisGVolume->getLogical();
	}

	return nullptr;
}

G4VPhysicalVolume* G4ObjectsFactory::getPhysicalFromMap(string vname, map<string, G4Volume*> *g4s) const
{

	if(g4s->find(vname) != g4s->end()) {
		auto thisGVolume = (*g4s)[vname];
		return thisGVolume->getPhysical();
	}

	return nullptr;
}

G4RotationMatrix* G4ObjectsFactory::getRotation(GVolume *s)
{
	G4RotationMatrix *rot = new G4RotationMatrix(G4ThreeVector(1, 0, 0),
																G4ThreeVector(0, 1, 0),
																G4ThreeVector(0, 0, 1));


	vector<string> rotDef = getStringVectorFromStringWithDelimiter(s->getRot(), ",");

	// default ordered rotation
	if(rotDef.size() == 3) {
		vector<double> rotPars = getG4NumbersFromStringVector(rotDef);
		rot->rotateX(rotPars[0]);
		rot->rotateY(rotPars[1]);
		rot->rotateZ(rotPars[2]);
		// custom ordered rotation
	} else if(rotDef.size() == 4) {
		vector<string> orderDefinitions = getStringVectorFromStringWithDelimiter(rotDef[0], " ");

		if(orderDefinitions[0] == "ordered:" || orderDefinitions[0] == "Ordered:") {
			string order = orderDefinitions[1];

			vector<double> rotPars = getG4NumbersFromStringVector({rotDef[1], rotDef[2], rotDef[3]});

			if(order == "xzy") {
				rot->rotateX(rotPars[0]);
				rot->rotateZ(rotPars[1]);
				rot->rotateY(rotPars[2]);
			} else if(order == "yxz") {
				rot->rotateY(rotPars[0]);
				rot->rotateX(rotPars[1]);
				rot->rotateZ(rotPars[2]);
			} else if(order == "yzx") {
				rot->rotateY(rotPars[0]);
				rot->rotateZ(rotPars[1]);
				rot->rotateX(rotPars[2]);
			} else if(order == "zxy") {
				rot->rotateZ(rotPars[0]);
				rot->rotateX(rotPars[1]);
				rot->rotateY(rotPars[2]);
			} else if(order == "zyx") {
				rot->rotateZ(rotPars[0]);
				rot->rotateY(rotPars[1]);
				rot->rotateX(rotPars[2]);
			} else {
				G4cerr << FATALERRORL << "ordered rotation <" << order << "> for " << s->getName() << " is wrong, it must be one of the following:"
				<< " xzy, yxz, yzx, zxy or zyx. Exiting." << G4endl;
				gexit(EC__G4WRONGORDEREDROTATION);
			}
		}
	}

	// tilt modifier (only xyz order)
	if(s->getTilt() != GSYSTEMNOMODIFIER) {
		vector<double> tiltVector = getG4NumbersFromString(s->getTilt(), true);

		if(tiltVector.size() == 3) {

			rot->rotateX(tiltVector[0]);
			rot->rotateY(tiltVector[1]);
			rot->rotateZ(tiltVector[2]);
		}
	}

	return rot;
}

G4ThreeVector G4ObjectsFactory::getPosition(GVolume *s)
{
	G4ThreeVector pos(0,0,0);

	vector<double> posVector = getG4NumbersFromString(s->getPos());
	if(posVector.size() == 3) {
		pos.setX(posVector[0]);
		pos.setY(posVector[1]);
		pos.setZ(posVector[2]);
	}

	// shift modifier
	if(s->getShift() != GSYSTEMNOMODIFIER) {
		vector<double> shiftVector = getG4NumbersFromString(s->getShift());
		if(shiftVector.size() == 3) {
			G4ThreeVector shift(shiftVector[0], shiftVector[1], shiftVector[2]);
			pos = pos + shift;
		}
	}

	return pos;
}

G4VisAttributes G4ObjectsFactory::createVisualAttributes(GVolume *s) {

	string vcolor = s->getColor();

	G4Colour g4Color;

	if(vcolor.size() == 6) {
		// if color is 6 digits then it's only rrggbb. Setting transparency to zero (1 in G4Colour)
		g4Color = G4Colour(strtol(vcolor.substr(0, 2).c_str(), NULL, 16)/255.0,
								 strtol(vcolor.substr(2, 2).c_str(), NULL, 16)/255.0,
								 strtol(vcolor.substr(4, 2).c_str(), NULL, 16)/255.0,
								 1);
	} else if(vcolor.size() == 7) {
		// Transparency 0 to 5 where 5=max transparency  (default is 0 if nothing is specified)
		g4Color = G4Colour(strtol(vcolor.substr(0, 2).c_str(), NULL, 16)/255.0,
								 strtol(vcolor.substr(2, 2).c_str(), NULL, 16)/255.0,
								 strtol(vcolor.substr(4, 2).c_str(), NULL, 16)/255.0,
								 1.0 - stof(vcolor.substr(6, 1))/5.0);
	}

	// new attributes, constructed from color
	G4VisAttributes g4attributes = G4VisAttributes(g4Color);

	// visibility and solid/wireframe style
	s->isVisible() ? g4attributes.SetVisibility(true) : g4attributes.SetVisibility(false);
	s->getStyle()  ? g4attributes.SetForceSolid(true) : g4attributes.SetForceWireframe(true);

	return g4attributes;
}
