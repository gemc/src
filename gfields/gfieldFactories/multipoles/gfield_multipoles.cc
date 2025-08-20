// CLHEP units
#include "CLHEP/Units/PhysicalConstants.h"

// geant4 headers
#include "G4ThreeVector.hh"

// gfields
#include "gfield_multipoles.h"
#include "gfieldConventions.h"

// gemv
// #include "gutilities.h"


// tells the DLL how to create a GFieldFactory in each plugin .so/.dylib
extern "C" GField* GFieldFactory(const std::shared_ptr<GOptions>& g) { return static_cast<GField*>(new GField_MultipolesFactory(g)); }


// for now this implementation follows gemc
// references of this implementation:
// - https://cds.cern.ch/record/1333874/files/1.pdf
// - https://uspas.fnal.gov/materials/12MSU/magnet_elements.pdf
// - https://cas.web.cern.ch/sites/default/files/lectures/bruges-2009/wolski-1.pdf
// notice strength is defined at a reference radius of 1m
void GField_MultipolesFactory::GetFieldValue(const double pos[3], G4double* bfield) const {
	// ======= Configuration / conventions =======
	// strength: Tesla at reference radius r0 for all multipole orders
	const double r0 = CLHEP::m; // reference radius; make this a member if you want

	// ======= Basic checks =======
	if (pole_number < 2 || (pole_number % 2) != 0) {
		log->error(ERR_WRONG_POLE_NUMBER,
		           "Pole number must be an even integer >= 2 (2=dipole,4=quadrupole,...)");
	}

	// ======= Positions and local frame =======
	const G4ThreeVector x0(pos[0], pos[1], pos[2]);          // query point (lab)
	const G4ThreeVector x1(origin[0], origin[1], origin[2]); // magnet origin (lab)

	// shift to magnet-centered coordinates and "unroll" the magnet by -rotation_angle
	G4ThreeVector p = x0 - x1;
	if (rotaxis == 0) p.rotateX(-rotation_angle);
	else if (rotaxis == 1) p.rotateY(-rotation_angle);
	else if (rotaxis == 2) p.rotateZ(-rotation_angle);

	// Identify transverse plane (u,v) ⟂ to the axis, and the axial coordinate w (unused here)
	double u = 0.0, v = 0.0;
	switch (rotaxis) {
	case 0: u = p.y();
		v = p.z();
		break; // axis = X ⇒ transverse plane = (Y,Z)
	case 1: u = p.z();
		v = p.x();
		break; // axis = Y ⇒ transverse plane = (Z,X)
	case 2: u = p.x();
		v = p.y();
		break; // axis = Z ⇒ transverse plane = (X,Y)
	default: break;
	}

	const double r   = std::hypot(u, v); // transverse radius
	const double phi = std::atan2(v, u); // azimuth in transverse plane

	// ======= Axial (solenoid-like) mode if explicitly requested =======
	if (longitudinal) {
		// Uniform axial field aligned with rotaxis; not a multipole.
		G4ThreeVector B_local(0, 0, 0);
		switch (rotaxis) {
		case 0: B_local.setX(strength);
			break;
		case 1: B_local.setY(strength);
			break;
		case 2: B_local.setZ(strength);
			break;
		default: break;
		}
		// Roll back to lab
		G4ThreeVector B_lab = B_local;
		if (rotaxis == 0) B_lab.rotateX(+rotation_angle);
		else if (rotaxis == 1) B_lab.rotateY(+rotation_angle);
		else if (rotaxis == 2) B_lab.rotateZ(+rotation_angle);

		bfield[0] = B_lab.x();
		bfield[1] = B_lab.y();
		bfield[2] = B_lab.z();

		log->info(2, "Axial field mode (solenoid-like). Strength: ", strength,
		          " T, Field: (", bfield[0], ", ", bfield[1], ", ", bfield[2], ")");
		return;
	}

	// ======= Transverse multipole (standard accelerator definition) =======
	const int n = pole_number / 2; // 1=dipole, 2=quadrupole, 3=sextupole, ...
	const int a = n - 1;           // power of r

	// r^a scaling with reference radius r0; for dipole (a=0) this is 1
	double ra = 1.0;
	if (a > 0) {
		// If r=0 and a>0, |B|=0 (for ideal multipoles).
		if (r == 0.0) {
			bfield[0] = bfield[1] = bfield[2] = 0.0;
			return;
		}
		ra = std::pow(r / r0, static_cast<double>(a));
	}

	// "Normal" multipole angular dependence:
	// Bu = strength * r^(n-1) * cos((n-1)*phi)
	// Bv = strength * r^(n-1) * sin((n-1)*phi)
	// This yields a constant transverse dipole for n=1.
	const double Bu = strength * ra * std::cos(a * phi);
	const double Bv = strength * ra * std::sin(a * phi);

	// Place (Bu,Bv) into the correct transverse components of B_local; axial component = 0
	G4ThreeVector B_local(0, 0, 0);
	switch (rotaxis) {
	case 0: B_local.setY(Bu);
		B_local.setZ(Bv);
		break; // axis X → (Y,Z)
	case 1: B_local.setZ(Bu);
		B_local.setX(Bv);
		break; // axis Y → (Z,X)
	case 2: B_local.setX(Bu);
		B_local.setY(Bv);
		break; // axis Z → (X,Y)
	default: break;
	}

	// Rotate (roll) back to lab
	G4ThreeVector B_lab = B_local;
	if (rotaxis == 0) B_lab.rotateX(+rotation_angle);
	else if (rotaxis == 1) B_lab.rotateY(+rotation_angle);
	else if (rotaxis == 2) B_lab.rotateZ(+rotation_angle);

	// Output
	bfield[0] = B_lab.x();
	bfield[1] = B_lab.y();
	bfield[2] = B_lab.z();

	log->info(2, "Pole Number: ", pole_number,
	          ", n: ", n,
	          ", Strength: ", strength,
	          ", Requested at: (", pos[0], ", ", pos[1], ", ", pos[2], ")",
	          ", Rotation angle: ", rotation_angle,
	          ", Rotation axis: ", rotaxis,
	          ", longitudinal: ", longitudinal,
	          ", Field: (", bfield[0], ", ", bfield[1], ", ", bfield[2], ")");
}


void GField_MultipolesFactory::load_field_definitions(GFieldDefinition gfd) {
	gfield_definitions = gfd;

	pole_number          = get_field_parameter_int("pole_number");
	origin[0]            = get_field_parameter_double("vx");
	origin[1]            = get_field_parameter_double("vy");
	origin[2]            = get_field_parameter_double("vz");
	rotation_angle       = get_field_parameter_double("rotation_angle");
	auto rot_axis_option = gfield_definitions.field_parameters["rotaxis"];
	longitudinal         = false;
	if (gfield_definitions.field_parameters["longitudinal"] == "true") {
		longitudinal = true;
		log->info(1, "Longitudinal field");
	}
	else { log->info(1, "Transverse field"); }

	if (rot_axis_option == "X" || rot_axis_option == "x") { rotaxis = 0; }
	else if (rot_axis_option == "Y" || rot_axis_option == "y") { rotaxis = 1; }
	else if (rot_axis_option == "Z" || rot_axis_option == "z") { rotaxis = 2; }
	else {
		log->error(ERR_WRONG_FIELD_ROTATION, "GField_MultipolesFactory::load_field_definitions: Rotation axis " + gfield_definitions.field_parameters["rotaxis"] +
		                                     " not supported. Exiting.");
	}
	log->info(1, "Rotation axis: ", rotaxis);
	strength = get_field_parameter_double("strength");
}
