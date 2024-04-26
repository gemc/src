// g4system
#include "g4NativeObjectsFactory.h"


// PRAGMA TODO: 
// to be completed - order is the same as https://geant4.web.cern.ch/geant4/UserDocumentation/UsersGuides/ForApplicationDeveloper/html/ch04.html


// returns description of geant4 constructor parameters
vector<string> G4NativeSystemFactory::descriptionsOfParameters(GVolume *s)
{
	vector<string> parsDescription;
	string type = s->getType();


	       if(type == "G4Box") {
		parsDescription.push_back({"half length in X", "Length"});
		parsDescription.push_back({"half length in Y", "Length"});
		parsDescription.push_back({"half length in Z", "Length"});
	} else if(type == "G4Tubs") {
		parsDescription.push_back({"Inner radius",       "Length"});
		parsDescription.push_back({"Outer radius",       "Length"});
		parsDescription.push_back({"Half length in z",   "Length"});
		parsDescription.push_back({"Starting Phi angle", "Angle"});
		parsDescription.push_back({"Delta Phi angle",    "Angle"});
	}








	//
	//	if(type == "Sphere")
	//	{
	//		dt[1] = "Length";
	//		dt[0] = "Inner radius";
	//		dtypes.push_back(dt);
	//		dt[0] = "Outer radius";
	//		dtypes.push_back(dt);
	//		dt[1] = "Angle";
	//		dt[0] = "Starting Phi angle of the segment";
	//		dtypes.push_back(dt);
	//		dt[0] = "Delta Phi angle of the segment";
	//		dtypes.push_back(dt);
	//		dt[0] = "Starting Theta angle of the segment";
	//		dtypes.push_back(dt);
	//		dt[0] = "Delta Theta angle of the segment";
	//		dtypes.push_back(dt);
	//	}
	//	if(type == "Trd")
	//	{
	//		dt[1] = "Length";
	//		dt[0] = "Half-length along x at the surface at -dz";
	//		dtypes.push_back(dt);
	//		dt[0] = "Half-length along x at the surface at +dz";
	//		dtypes.push_back(dt);
	//		dt[0] = "Half-length along y at the surface at -dz";
	//		dtypes.push_back(dt);
	//		dt[0] = "Half-length along y at the surface at +dz";
	//		dtypes.push_back(dt);
	//		dt[0] = "dz: Half-length along z axis";
	//		dtypes.push_back(dt);
	//	}
	//	if(type == "Cons")
	//	{
	//		dt[1] = "Length";
	//		dt[0] = "Inner radius at start";
	//		dtypes.push_back(dt);
	//		dt[0] = "Outer radius at start";
	//		dtypes.push_back(dt);
	//		dt[0] = "Inner radius at end";
	//		dtypes.push_back(dt);
	//		dt[0] = "Outer radius at end";
	//		dtypes.push_back(dt);
	//		dt[0] = "Half length in z";
	//		dtypes.push_back(dt);
	//		dt[1] = "Angle";
	//		dt[0] = "Starting Phi angle";
	//		dtypes.push_back(dt);
	//		dt[0] = "Delta Phi angle";
	//		dtypes.push_back(dt);
	//	}
	//	if(type == "G4Trap")
	//	{
	//		dt[1] = "Length";
	//		dt[0] = "Half z length ";
	//		dtypes.push_back(dt);
	//		dt[1] = "Angle";
	//		dt[0] = "Polar angle of the line joining the centres of the faces";
	//		dtypes.push_back(dt);
	//		dt[0] = "Azimuthal angle of the line joining the centre of the face";
	//		dtypes.push_back(dt);
	//		dt[1] = "Length";
	//		dt[0] = "Half y length at -pDz";
	//		dtypes.push_back(dt);
	//		dt[0] = "Half x length of the side at y=-pDy1, -pDz";
	//		dtypes.push_back(dt);
	//		dt[0] = "Half x length of the side at y=+pDy1, -pDz";
	//		dtypes.push_back(dt);
	//		dt[1] = "Angle";
	//		dt[0] = "Angle to the y axis from the centre (lower endcap) ";
	//		dtypes.push_back(dt);
	//		dt[1] = "Length";
	//		dt[0] = "Half y length at +pDz";
	//		dtypes.push_back(dt);
	//		dt[0] = "Half x length of the side at y=-pDy2, +pDz";
	//		dtypes.push_back(dt);
	//		dt[0] = "Half x length of the side at y=+pDy2, +pDz";
	//		dtypes.push_back(dt);
	//		dt[1] = "Angle";
	//		dt[0] = "Angle to the y axis from the centre (upper endcap) ";
	//		dtypes.push_back(dt);
	//	}
	//
	//	if(type == "G4EllipticalTube")
	//	{
	//		dt[1]="Lenght";
	//		dt[0]="Half length x";
	//		dtypes.push_back(dt);
	//		dt[1]="Lenght";
	//		dt[0]="Half length y";
	//		dtypes.push_back(dt);
	//		dt[1]="Lenght";
	//		dt[0]="Half length z";
	//		dtypes.push_back(dt);
	//	}
	//
	//	if(type == "Hype")
	//	{
	//		dt[1]="Length";
	//		dt[0]="Inner radius";
	//		dtypes.push_back(dt);
	//		dt[0] = "Outer radius";
	//		dtypes.push_back(dt);
	//		dt[1] = "Angle";
	//		dt[0] = "Inner stereo angle";
	//		dtypes.push_back(dt);
	//		dt[0] = "Outer stereo angle";
	//		dtypes.push_back(dt);
	//		dt[1]="Length";
	//		dt[0] = "Half length in z";
	//		dtypes.push_back(dt);
	//	}
	//
	//	if(type == "Parallelepiped")
	//	{
	//		dt[1]="Length";
	//		dt[0]="Half length in x";
	//		dtypes.push_back(dt);
	//		dt[0] = "Half length in y";
	//		dtypes.push_back(dt);
	//		dt[0] = "Half length in z";
	//		dtypes.push_back(dt);
	//		dt[1] = "Angle";
	//		dt[0] = "Angle formed by the y axis and the plane joining the centre of the faces parallel to the z-x plane at -dy and +dy" ;
	//		dtypes.push_back(dt);
	//		dt[0] = "Polar angle of the line joining the centres of the faces at -dz and +dz in z ";
	//		dtypes.push_back(dt);
	//		dt[0] = "Azimuthal angle of the line joining the centres of the faces at -dz and +dz in z ";
	//		dtypes.push_back(dt);
	//	}
	//
	//	if(type == "Torus")
	//	{
	//		dt[1]="Length";
	//		dt[0]="Inner radius";
	//		dtypes.push_back(dt);
	//		dt[0] = "Outer radius";
	//		dtypes.push_back(dt);
	//		dt[0] = "Swept radius of torus";
	//		dtypes.push_back(dt);
	//		dt[1] = "Angle";
	//		dt[0] = "Starting Phi angle";
	//		dtypes.push_back(dt);
	//		dt[0] = "Outer stereo angle";
	//		dtypes.push_back(dt);
	//	}
	//
	//	if(type == "Ellipsoid")
	//	{
	//		dt[1]="Length";
	//		dt[0]="Semiaxis in X";
	//		dtypes.push_back(dt);
	//		dt[0] = "Semiaxis in Y ";
	//		dtypes.push_back(dt);
	//		dt[0] = "Semiaxis in Z ";
	//		dtypes.push_back(dt);
	//		dt[0] = "lower cut plane level, z";
	//		dtypes.push_back(dt);
	//		dt[0] = "upper cut plane level, z";
	//		dtypes.push_back(dt);
	//	}

	return parsDescription;
}

