/**
 * \file   buildSolid.cc
 * @ingroup g4system_geometry
 * \brief  Implementation of G4NativeSystemFactory::buildSolid().
 */

// g4system
#include "g4NativeObjectsFactory.h"
#include "g4systemConventions.h"

// guts
#include "gutilities.h"

// geant4
#include "G4Box.hh"
#include "G4Sphere.hh"
#include "G4Torus.hh"
#include "G4Tubs.hh"
#include "G4CutTubs.hh"
#include "G4Cons.hh"
#include "G4Para.hh"
#include "G4Trap.hh"
#include "G4Trd.hh"
#include "G4Polycone.hh"
#include "G4Polyhedra.hh"
#include "G4Paraboloid.hh"
#include "G4EllipticalTube.hh"
#include "G4UnionSolid.hh"
#include "G4SubtractionSolid.hh"
#include "G4IntersectionSolid.hh"

// Build a native Geant4 solid for the given volume definition.
// Header documentation is authoritative; this implementation comment is intentionally brief.
G4VSolid* G4NativeSystemFactory::buildSolid(const GVolume*                              s,
                                            std::unordered_map<std::string, G4Volume*>* g4s) {
	std::string g4name = s->getG4Name();

	log->info(2, className(), "G4NativeSystemFactory::buildSolid for ", g4name);

	// Dependencies must be satisfied before constructing a solid (copy/boolean operands).
	if (!checkSolidDependencies(s, g4s)) return nullptr;

	// Locate or allocate the wrapper used to cache solid/logical/physical pointers.
	auto thisG4Volume = getOrCreateG4Volume(g4name, g4s);

	// Record the volume's own frame rotation and position: when this solid is the
	// second operand of a boolean operation, they define the relative transform.
	{
		auto* rot = getRotation(s);
		thisG4Volume->setSolidPlacement(*rot, getPosition(s));
		delete rot;
	}

	// Solid exists, return it.
	if (thisG4Volume->getSolid() != nullptr) return thisG4Volume->getSolid();

	// If this is a copy, reuse the source solid if available.
	std::string copyOf = s->getCopyOf();
	if (copyOf != "" && copyOf != UNINITIALIZEDSTRINGQUANTITY) {
		auto gsystem      = s->getSystem();
		auto volume_copy  = gsystem + "/" + copyOf;
		auto thisG4Volume = getOrCreateG4Volume(volume_copy, g4s);
		if (thisG4Volume->getSolid() != nullptr) return thisG4Volume->getSolid();
	}

	// Boolean solids use already-built operand solids.
	std::string solidsOpr = s->getSolidsOpr();
	if (solidsOpr != "" && solidsOpr != UNINITIALIZEDSTRINGQUANTITY) {
		std::vector<std::string> solidOperations = gutilities::getStringVectorFromString(solidsOpr);
		if (solidOperations.size() == 3) {
			auto resolveOperandName = [s, g4s](const std::string& operand) -> std::string {
				if (getSolidFromMap(operand, g4s) != nullptr) return operand;
				return s->getSystem() + "/" + operand;
			};

			auto leftName  = resolveOperandName(solidOperations[0]);
			auto rightName = resolveOperandName(solidOperations[2]);
			auto left      = getSolidFromMap(leftName, g4s);
			auto right     = getSolidFromMap(rightName, g4s);
			if (left == nullptr || right == nullptr) return nullptr;

			// GEMC2 `Operation:` convention (clas12Tags detector.cc): the first solid
			// is taken at identity; the second is rotated by the inverse of its own
			// frame rotation, then translated by its own position.
			auto             rightWrapper = getOrCreateG4Volume(rightName, g4s);
			G4RotationMatrix rotate       = rightWrapper->getSolidRotation();
			G4ThreeVector    translate    = rightWrapper->getSolidTranslation();
			G4RotationMatrix invRot       = rotate.invert();
			G4Transform3D    transf1(invRot, G4ThreeVector(0, 0, 0));
			G4Transform3D    transf2(G4RotationMatrix(), translate);
			G4Transform3D    transform = transf2 * transf1;

			if (solidOperations[1] == "+") {
				thisG4Volume->setSolid(new G4UnionSolid(g4name, left, right, transform), log);
			}
			else if (solidOperations[1] == "-") {
				thisG4Volume->setSolid(new G4SubtractionSolid(g4name, left, right, transform), log);
			}
			else if (solidOperations[1] == "*") {
				thisG4Volume->setSolid(new G4IntersectionSolid(g4name, left, right, transform), log);
			}
			else {
				log->error(ERR_G4PARAMETERSMISMATCH,
				           "The boolean constructor of <", g4name, "> uses unsupported operator <",
				           solidOperations[1], ">. Use +, -, or *.");
			}
			return thisG4Volume->getSolid();
		}
		log->error(ERR_G4PARAMETERSMISMATCH,
		           "The boolean constructor of <", g4name, "> must be: left operator right.");
		return nullptr;
	}

	// Parse and validate parameters for the requested primitive.
	std::vector<double> pars = checkAndReturnParameters(s);

	std::string type = s->getType();

	if (type == "G4Box") {
		thisG4Volume->setSolid(new G4Box(g4name,  // name
		                                 pars[0], // half-length in X
		                                 pars[1], // half-length in Y
		                                 pars[2]  // half-length in Z
		                       ), log);
		return thisG4Volume->getSolid();
	}
	else if (type == "G4Tubs") {
		thisG4Volume->setSolid(new G4Tubs(g4name,  // name
		                                  pars[0], // Inner radius
		                                  pars[1], // Outer radius
		                                  pars[2], // half-length in z
		                                  pars[3], // Starting phi angle
		                                  pars[4]  // Delta Phi angle
		                       ), log);
		return thisG4Volume->getSolid();
	}
	else if (type == "G4Sphere") {
		thisG4Volume->setSolid(new G4Sphere(g4name,  // name
		                                    pars[0], // Inner radius
		                                    pars[1], // Outer radius
		                                    pars[2], // Starting phi angle
		                                    pars[3], // Delta Phi angle
		                                    pars[4], // Starting delta angle
		                                    pars[5]  // Delta delta angle
		                       ), log);
		return thisG4Volume->getSolid();
	}
	else if (type == "G4Torus") {
		thisG4Volume->setSolid(new G4Torus(g4name,  // name
		                                    pars[0], // Inside radius of the torus tube
		                                    pars[1], // Outside radius of the torus tube
		                                    pars[2], // Swept radius of the torus
		                                    pars[3], // Starting phi angle
		                                    pars[4]  // Delta phi angle
		                       ), log);
		return thisG4Volume->getSolid();
	}
	else if (type == "G4CutTubs") {
		thisG4Volume->setSolid(new G4CutTubs(g4name,                                   // name
		                                     pars[0],                                  // Inner radius
		                                     pars[1],                                  // Outer radius
		                                     pars[2],                                  // half-length in z
		                                     pars[3],                                  // Starting phi angle
		                                     pars[4],                                  // Delta Phi angle
		                                     G4ThreeVector(pars[5], pars[6], pars[7]), // Outside Normal at -z
		                                     G4ThreeVector(pars[8], pars[9], pars[10]) // Outside Normal at +z
		                       ), log);
		return thisG4Volume->getSolid();
	}
	else if (type == "G4Cons") {
		thisG4Volume->setSolid(new G4Cons(g4name,  // name
		                                  pars[0], // Inside radius at -pDz
		                                  pars[1], // Outside radius at -pDz
		                                  pars[2], // Inside radius at +pDz
		                                  pars[3], // Outside radius at +pDz
		                                  pars[4], // half-length in z
		                                  pars[5], // Starting phi angle
		                                  pars[6]  // Delta Phi angle
		                       ), log);
		return thisG4Volume->getSolid();
	}
	else if (type == "G4Para") {
		thisG4Volume->setSolid(new G4Para(g4name,  // name
		                                  pars[0], // half-length in x
		                                  pars[1], // half-length in y
		                                  pars[2], // half-length in z
		                                  pars[3],
		                                  // Angle formed by the y axis and by the plane joining the center of the faces parallel to the z-x plane at -dy and +dy
		                                  pars[4],
		                                  // Polar angle of the line joining the center of the faces at -dz and +dz in z
		                                  pars[5]
		                                  // Azimuthal angle of the line joining the center of the faces at -dz and +dz in z
		                       ), log);
		return thisG4Volume->getSolid();
	}
	else if (type == "G4Trd") {
		thisG4Volume->setSolid(new G4Trd(g4name,  // name
		                                 pars[0], // Half-length along x at the surface positioned at -dz
		                                 pars[1], // Half-length along x at the surface positioned at +dz
		                                 pars[2], // Half-length along y at the surface positioned at -dz
		                                 pars[3], // Half-length along y at the surface positioned at +dz
		                                 pars[4]  // Half-length along z axis
		                       ), log);
		return thisG4Volume->getSolid();
	}
	else if (type == "G4Trap") {
		// G4Trap supports multiple constructor layouts; parameter count decides which one is used.
		if (pars.size() == 4) {
			thisG4Volume->setSolid(new G4Trap(g4name,  // name
			                                  pars[0], // Length along Z
			                                  pars[1], // Length along Y
			                                  pars[2], // Length along X wider side
			                                  pars[3]  // Length along X at the narrower side (plTX<=pX)
			                       ), log);
		}
		else if (pars.size() == 11) {
			thisG4Volume->setSolid(new G4Trap(g4name,  // name
			                                  pars[0], // Half Z length - distance from the origin to the bases
			                                  pars[1],
			                                  // Polar angle of the line joining the center of the bases at -/+pDz
			                                  pars[2], // Azimuthal angle of the same line
			                                  pars[3], // Half Y length of the base at -pDz
			                                  pars[4], // Half Y length of the base at +pDz
			                                  pars[5], // Half X length at smaller Y of the base at -pDz
			                                  pars[6], // Half X length at bigger Y of the base at -pDz
			                                  pars[7], // Half X length at smaller Y of the base at +pDz
			                                  pars[8], // Half X length at bigger y of the base at +pDz
			                                  pars[9], // Angle between Y-axis and center line at -pDz
			                                  pars[10] // Angle between Y-axis and center line at +pDz
			                       ), log);
		}
		else if (pars.size() == 24) {
			G4ThreeVector pt[8];
			pt[0] = G4ThreeVector(pars[0], pars[1], pars[2]);
			pt[1] = G4ThreeVector(pars[3], pars[4], pars[5]);
			pt[2] = G4ThreeVector(pars[6], pars[7], pars[8]);
			pt[3] = G4ThreeVector(pars[9], pars[10], pars[11]);
			pt[4] = G4ThreeVector(pars[12], pars[13], pars[14]);
			pt[5] = G4ThreeVector(pars[15], pars[16], pars[17]);
			pt[6] = G4ThreeVector(pars[18], pars[19], pars[20]);
			pt[7] = G4ThreeVector(pars[21], pars[22], pars[23]);

			thisG4Volume->setSolid(new G4Trap(g4name, pt), log);
		}
		else {
			log->error(ERR_G4PARAMETERSMISMATCH,
			           "The constructor of <", g4name, "> must have 4, 11 or 24 parameters",
			           " see https://geant4-userdoc.web.cern.ch/UsersGuides/ForApplicationDeveloper/html/Detector/Geometry/geomSolids.html");
		}
		return thisG4Volume->getSolid();
	}
	else if (type == "G4Polycone") {
		double phistart = pars[0];
		double phitotal = pars[1];
		int    zplanes  = static_cast<int>(pars[2]);

		// Allocate arrays (data is copied by G4Polycone during construction).
		auto zPlane = std::make_unique<double[]>(zplanes);
		auto rInner = std::make_unique<double[]>(zplanes);
		auto rOuter = std::make_unique<double[]>(zplanes);

		for (int zpl = 0; zpl < zplanes; ++zpl) {
			zPlane[zpl] = pars[3 + 0 * zplanes + zpl];
			rInner[zpl] = pars[3 + 1 * zplanes + zpl];
			rOuter[zpl] = pars[3 + 2 * zplanes + zpl];
		}

		thisG4Volume->setSolid(new G4Polycone(g4name,   // name
		                                      phistart, // Initial Phi starting angle
		                                      phitotal, // Total Phi angle
		                                      zplanes,  // Number of z planes
		                                      zPlane.get(),
		                                      rInner.get(),
		                                      rOuter.get()
		                       ), log);
		return thisG4Volume->getSolid();
	}
	else if (type == "G4Polyhedra") {
		// GEMC2 "Pgon" parameter order: phiStart, phiTotal, numSides, numZPlanes,
		// then rInner[], rOuter[], zPlane[].
		double phistart = pars[0];
		double phitotal = pars[1];
		int    numSides = static_cast<int>(pars[2]);
		int    zplanes  = static_cast<int>(pars[3]);

		if (numSides < 1 || static_cast<int>(pars.size()) != 4 + 3 * zplanes) {
			log->error(ERR_G4PARAMETERSMISMATCH,
			           "The constructor of <", g4name, "> must have numSides >= 1 and ",
			           4 + 3 * zplanes, " parameters (4 + 3 x numZPlanes), we got ", pars.size());
		}

		// Allocate arrays (data is copied by G4Polyhedra during construction).
		auto zPlane = std::make_unique<double[]>(zplanes);
		auto rInner = std::make_unique<double[]>(zplanes);
		auto rOuter = std::make_unique<double[]>(zplanes);

		for (int zpl = 0; zpl < zplanes; ++zpl) {
			rInner[zpl] = pars[4 + 0 * zplanes + zpl];
			rOuter[zpl] = pars[4 + 1 * zplanes + zpl];
			zPlane[zpl] = pars[4 + 2 * zplanes + zpl];
		}

		thisG4Volume->setSolid(new G4Polyhedra(g4name,   // name
		                                       phistart, // Initial Phi starting angle
		                                       phitotal, // Total Phi angle
		                                       numSides, // Number of sides
		                                       zplanes,  // Number of z planes
		                                       zPlane.get(),
		                                       rInner.get(),
		                                       rOuter.get()
		                       ), log);
		return thisG4Volume->getSolid();
	}
	else if (type == "G4Paraboloid") {
		thisG4Volume->setSolid(new G4Paraboloid(g4name, // name
		                                        pars[0], // half-length in z
		                                        pars[1], // radius at -dz
		                                        pars[2]  // radius at +dz
		                       ), log);
		return thisG4Volume->getSolid();
	}
	else if (type == "G4EllipticalTube") {
		thisG4Volume->setSolid(new G4EllipticalTube(g4name, // name
		                                            pars[0], // half length in x
		                                            pars[1], // half length in y
		                                            pars[2]  // half length in z
		                       ), log);
		return thisG4Volume->getSolid();
	}
	else {
		log->error(ERR_G4SOLIDTYPENOTFOUND,
		           "The constructor of <", g4name, "> uses an unknown solid type <", type,
		           ">. See Geant4 manual for supported primitives.");
	}
	return nullptr;
}
