// dladdr (used to locate this plugin's own path) needs _GNU_SOURCE on glibc.
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

// plugin header
#include "gfield_asciimap.h"

// gemc gfield framework
#include "gfieldConventions.h"

// geant4 / CLHEP
#include "G4ThreeVector.hh"
#include "CLHEP/Units/SystemOfUnits.h"

// c++
#include <cmath>
#include <cstdlib>
#include <dlfcn.h>
#include <fstream>
#include <sstream>

using namespace CLHEP;

// Tells the loader how to create a GField in this plugin .so/.dylib.
extern "C" GField* GFieldFactory(const std::shared_ptr<GOptions>& g) {
	return static_cast<GField*>(new GField_AsciiMapFactory(g));
}


// ---------------------------------------------------------------------------------------------------
// Configuration helpers
// ---------------------------------------------------------------------------------------------------

std::string GField_AsciiMapFactory::param_string(const std::string& key, const std::string& dflt) const {
	auto it = gfield_definitions.field_parameters.find(key);
	return (it != gfield_definitions.field_parameters.end() && !it->second.empty()) ? it->second : dflt;
}

double GField_AsciiMapFactory::param_g4number(const std::string& key, const std::string& dflt) const {
	return gutilities::getG4Number(param_string(key, dflt));
}

std::string GField_AsciiMapFactory::field_maps_directory() const {
	// Locate this plugin's own shared object on disk and point at the sibling "fields" directory
	// (<plugin_dir>/../fields), the layout produced by `meson install`. No environment variable is used.
	Dl_info info;
	if (dladdr(reinterpret_cast<const void*>(&GFieldFactory), &info) != 0 && info.dli_fname != nullptr) {
		const std::string plugin_path = info.dli_fname;
		const auto        slash       = plugin_path.find_last_of('/');
		if (slash != std::string::npos) { return plugin_path.substr(0, slash) + "/../fields"; }
	}
	log->warning("GField_AsciiMapFactory: could not resolve plugin path; using ./fields for the map.");
	return "fields";
}

int GField_AsciiMapFactory::axis_of_coordinate(const std::string& name) const {
	switch (symmetry) {
	case Symmetry::dipole_x:
	case Symmetry::dipole_y:
	case Symmetry::dipole_z:
		if (name == "longitudinal") return 0;
		if (name == "transverse") return 1;
		return -1;
	case Symmetry::cyl_x:
	case Symmetry::cyl_y:
	case Symmetry::cyl_z:
		if (name == "transverse") return 0;
		if (name == "longitudinal") return 1;
		return -1;
	case Symmetry::phi_segmented:
		if (name == "azimuthal") return 0;
		if (name == "transverse") return 1;
		if (name == "longitudinal") return 2;
		return -1;
	case Symmetry::cartesian_3d:
	case Symmetry::cartesian_3d_quadrant:
		if (name == "X") return 0;
		if (name == "Y") return 1;
		if (name == "Z") return 2;
		return -1;
	}
	return -1;
}


// ---------------------------------------------------------------------------------------------------
// Loading
// ---------------------------------------------------------------------------------------------------

void GField_AsciiMapFactory::load_coordinate(const std::string& key) {
	const std::string value = param_string(key, "");
	if (value.empty()) {
		log->error(ERR_WRONG_COORDINATE_DEF,
		           "GField_AsciiMapFactory: missing coordinate <", key, "> for field <",
		           gfield_definitions.name, ">.");
	}

	// Split "name, npoints, min, max" on commas.
	const auto tokens = gutilities::getStringVectorFromStringWithDelimiter(value, ",");
	if (tokens.size() != 4) {
		log->error(ERR_WRONG_COORDINATE_DEF,
		           "GField_AsciiMapFactory: coordinate <", key, "> must be \"name, npoints, min, max\". Got <",
		           value, ">.");
	}

	const std::string name    = tokens[0];
	const int         npoints = std::stoi(tokens[1]);
	const double      min     = gutilities::getG4Number(tokens[2]);
	const double      max     = gutilities::getG4Number(tokens[3]);

	const int axis = axis_of_coordinate(name);
	if (axis < 0) {
		log->error(ERR_WRONG_COORDINATE_DEF,
		           "GField_AsciiMapFactory: coordinate name <", name, "> is not valid for symmetry <",
		           gfield_definitions.field_parameters["symmetry"], ">.");
	}
	if (npoints < 2) {
		log->error(ERR_WRONG_COORDINATE_DEF,
		           "GField_AsciiMapFactory: coordinate <", name, "> needs at least 2 points; got ", npoints, ".");
	}

	np[axis]       = static_cast<unsigned>(npoints);
	startMap[axis] = min;
	endMap[axis]   = max;
	cellSize[axis] = (max - min) / (npoints - 1);

	// The map-file column for this coordinate is read in the same unit used in the min/max expression,
	// so a bare column value is converted back to Geant4 units with that factor.
	const std::string unit = [&]() {
		for (const std::string& expr : {tokens[3], tokens[2]}) {
			const auto star = expr.find('*');
			if (star != std::string::npos) {
				return gutilities::removeLeadingAndTrailingSpacesFromString(expr.substr(star + 1));
			}
		}
		return std::string("mm");
	}();

	columns.push_back(Column{axis, gutilities::getG4Number(1.0, unit)});
}

void GField_AsciiMapFactory::load_map_file() {
	const std::string map_name = param_string("map", "");
	if (map_name.empty()) {
		log->error(ERR_MAP_FILE_NOT_FOUND,
		           "GField_AsciiMapFactory: no 'map' file given for field <", gfield_definitions.name, ">.");
	}

	// Resolve the map file. An explicit path (containing '/') is used as-is. Otherwise the lookup order
	// is: the explicit `dir` parameter, then the directory of the YAML that defined the field (so a plain
	// .yaml run from its own directory or referenced by absolute path just works), then the `fields`
	// directory installed next to the plugin.
	std::string path = map_name;
	if (map_name.find('/') == std::string::npos) {
		std::vector<std::string> candidates;
		if (const std::string dir = param_string("dir", ""); !dir.empty()) { candidates.push_back(dir); }
		if (!gfield_definitions.config_dir.empty()) { candidates.push_back(gfield_definitions.config_dir); }
		candidates.push_back(field_maps_directory());

		path.clear();
		for (const auto& dir : candidates) {
			const std::string trial = dir + "/" + map_name;
			if (std::ifstream(trial).good()) { path = trial; break; }
		}
		if (path.empty()) {
			log->error(ERR_MAP_FILE_NOT_FOUND,
			           "GField_AsciiMapFactory: cannot find map <", map_name, "> for field <",
			           gfield_definitions.name, "> in dir/config/plugin locations.");
		}
	}

	std::ifstream in(path);
	if (!in.good()) {
		log->error(ERR_MAP_FILE_NOT_FOUND, "GField_AsciiMapFactory: cannot open map file <", path, ">.");
	}
	log->info(1, "Loading ASCII field map <", path, "> with symmetry <",
	          gfield_definitions.field_parameters["symmetry"], ">.");

	// Total grid points and field-value scaling.
	std::size_t total = np[0];
	for (int d = 1; d < ndim; ++d) { total *= np[d]; }

	const double field_scale = param_g4number("scale", "1") *
	                           gutilities::getG4Number(1.0, param_string("field_unit", "gauss"));

	B1.assign(total, 0.0f);
	if (ncomp >= 2) { B2.assign(total, 0.0f); }
	if (ncomp >= 3) { B3.assign(total, 0.0f); }

	const double tolerance = 0.001; // relative cell-position tolerance for the grid-consistency check

	std::size_t read_points = 0;
	std::string line;
	int         line_number = 0;
	while (std::getline(in, line)) {
		++line_number;
		const auto trimmed = gutilities::removeLeadingAndTrailingSpacesFromString(line);
		if (trimmed.empty() || trimmed[0] == '#') { continue; }

		std::istringstream tokens(trimmed);

		// Read the coordinate columns (file-column order), convert them to Geant4 units and turn each
		// into its canonical axis index.
		unsigned grid_index[3] = {0, 0, 0};
		bool     line_ok       = true;
		for (int c = 0; c < ndim; ++c) {
			double raw = 0.0;
			if (!(tokens >> raw)) { line_ok = false; break; }
			const double coord = raw * columns[c].unitFactor;
			const int    axis  = columns[c].axis;
			const unsigned i    = static_cast<unsigned>(
				std::floor((coord - startMap[axis] + cellSize[axis] / 2) / cellSize[axis]));
			grid_index[axis] = i;

			const double expected = startMap[axis] + i * cellSize[axis];
			if (cellSize[axis] != 0.0 && std::fabs(expected - coord) > tolerance * std::fabs(cellSize[axis])) {
				log->warning("GField_AsciiMapFactory: ", path, ":", line_number,
				             " axis ", axis, " value ", coord, " does not match grid point ", expected, ".");
			}
		}

		double comp[3] = {0.0, 0.0, 0.0};
		for (int k = 0; k < ncomp && line_ok; ++k) {
			if (!(tokens >> comp[k])) { line_ok = false; }
		}
		if (!line_ok) {
			log->error(ERR_MAP_FILE_NOT_FOUND, "GField_AsciiMapFactory: ", path, ":", line_number,
			           " has fewer than ", ndim + ncomp, " numbers.");
		}

		std::size_t flat = (ndim == 2) ? idx2(grid_index[0], grid_index[1])
		                                : idx3(grid_index[0], grid_index[1], grid_index[2]);
		if (flat >= total) {
			log->warning("GField_AsciiMapFactory: ", path, ":", line_number, " maps outside the grid; skipped.");
			continue;
		}

		B1[flat] = static_cast<float>(comp[0] * field_scale);
		if (ncomp >= 2) { B2[flat] = static_cast<float>(comp[1] * field_scale); }
		if (ncomp >= 3) { B3[flat] = static_cast<float>(comp[2] * field_scale); }
		++read_points;
	}

	if (read_points != total) {
		log->warning("GField_AsciiMapFactory: ", path, " provided ", read_points,
		             " points but the grid expects ", total, ".");
	}
	log->info(1, "ASCII field map <", gfield_definitions.name, "> loaded: ", read_points, " points.");
}

void GField_AsciiMapFactory::load_field_definitions(GFieldDefinition gfd) {
	gfield_definitions = gfd;

	// Decode the symmetry once. ndim/ncomp follow from it.
	const std::string sym = param_string("symmetry", "");
	if (sym == "dipole-x") { symmetry = Symmetry::dipole_x; ndim = 2; ncomp = 1; }
	else if (sym == "dipole-y") { symmetry = Symmetry::dipole_y; ndim = 2; ncomp = 1; }
	else if (sym == "dipole-z") { symmetry = Symmetry::dipole_z; ndim = 2; ncomp = 1; }
	else if (sym == "cylindrical-x") { symmetry = Symmetry::cyl_x; ndim = 2; ncomp = 2; }
	else if (sym == "cylindrical-y") { symmetry = Symmetry::cyl_y; ndim = 2; ncomp = 2; }
	else if (sym == "cylindrical-z") { symmetry = Symmetry::cyl_z; ndim = 2; ncomp = 2; }
	else if (sym == "phi-segmented") { symmetry = Symmetry::phi_segmented; ndim = 3; ncomp = 3; }
	else if (sym == "cartesian_3D") { symmetry = Symmetry::cartesian_3d; ndim = 3; ncomp = 3; }
	else if (sym == "cartesian_3D_quadrant") { symmetry = Symmetry::cartesian_3d_quadrant; ndim = 3; ncomp = 3; }
	else {
		log->error(ERR_WRONG_FIELD_SYMMETRY,
		           "GField_AsciiMapFactory: unknown symmetry <", sym, "> for field <",
		           gfield_definitions.name, ">.");
	}

	// Grid coordinates, in map-file column order.
	columns.clear();
	columns.reserve(ndim);
	load_coordinate("coordinate1");
	load_coordinate("coordinate2");
	if (ndim == 3) { load_coordinate("coordinate3"); }

	// Interpolation.
	linear = (param_string("interpolation", "linear") != "none");

	// Overall placement.
	mapOrigin[0]   = param_g4number("vx", "0");
	mapOrigin[1]   = param_g4number("vy", "0");
	mapOrigin[2]   = param_g4number("vz", "0");
	mapRotation[0] = param_g4number("rx", "0*deg");
	mapRotation[1] = param_g4number("ry", "0*deg");
	mapRotation[2] = param_g4number("rz", "0*deg");
	sinAlpha = std::sin(mapRotation[0]); cosAlpha = std::cos(mapRotation[0]);
	sinBeta  = std::sin(mapRotation[1]); cosBeta  = std::cos(mapRotation[1]);
	sinGamma = std::sin(mapRotation[2]); cosGamma = std::cos(mapRotation[2]);

	load_map_file();
}


// ---------------------------------------------------------------------------------------------------
// Field evaluation
// ---------------------------------------------------------------------------------------------------

void GField_AsciiMapFactory::GetFieldValue(const double pos[3], G4double* bfield) const {
	bfield[0] = bfield[1] = bfield[2] = 0.0;

	if (std::isnan(pos[0]) || std::isnan(pos[1]) || std::isnan(pos[2])) {
		log->warning("GField_AsciiMapFactory: field coordinates requested are nan.");
		return;
	}

	// Shift to the map frame.
	const double x[3] = {pos[0] - mapOrigin[0], pos[1] - mapOrigin[1], pos[2] - mapOrigin[2]};

	switch (symmetry) {
	case Symmetry::dipole_x:
	case Symmetry::dipole_y:
	case Symmetry::dipole_z:
		value_dipole(x, bfield);
		break;
	case Symmetry::cyl_x:
	case Symmetry::cyl_y:
	case Symmetry::cyl_z:
		value_cylindrical(x, bfield);
		break;
	case Symmetry::phi_segmented:
		value_phi_segmented(x, bfield);
		break;
	case Symmetry::cartesian_3d:
	case Symmetry::cartesian_3d_quadrant:
		value_cartesian3d(x, bfield);
		break;
	}

	log->info(2, "ASCII field <", gfield_definitions.name, "> at (",
	          pos[0] / cm, ", ", pos[1] / cm, ", ", pos[2] / cm, ") cm = (",
	          bfield[0] / gauss, ", ", bfield[1] / gauss, ", ", bfield[2] / gauss, ") gauss");
}


void GField_AsciiMapFactory::value_dipole(const double x[3], double* bfield) const {
	// Axis 0 = longitudinal, axis 1 = transverse.
	double LC = 0.0, TC = 0.0;
	if (symmetry == Symmetry::dipole_z) { TC = std::fabs(x[0]); LC = x[1]; }
	else if (symmetry == Symmetry::dipole_x) { TC = std::fabs(x[1]); LC = x[2]; }
	else /* dipole_y */ { TC = std::fabs(x[0]); LC = x[2]; }

	if (LC < startMap[0] || TC < startMap[1]) { return; }
	unsigned IL = static_cast<unsigned>(std::floor((LC - startMap[0]) / cellSize[0]));
	unsigned IT = static_cast<unsigned>(std::floor((TC - startMap[1]) / cellSize[1]));
	if (IL >= np[0] - 1 || IT >= np[1] - 1) { return; }

	double b = 0.0;
	if (!linear) {
		if (std::fabs(startMap[0] + IL * cellSize[0] - LC) > std::fabs(startMap[0] + (IL + 1) * cellSize[0] - LC)) IL++;
		if (std::fabs(startMap[1] + IT * cellSize[1] - TC) > std::fabs(startMap[1] + (IT + 1) * cellSize[1] - TC)) IT++;
		b = B1[idx2(IL, IT)];
	}
	else {
		const double xlr = (LC - (startMap[0] + IL * cellSize[0])) / cellSize[0];
		const double xtr = (TC - (startMap[1] + IT * cellSize[1])) / cellSize[1];
		const double b10 = B1[idx2(IL, IT)] * (1.0 - xtr) + B1[idx2(IL, IT + 1)] * xtr;
		const double b11 = B1[idx2(IL + 1, IT)] * (1.0 - xtr) + B1[idx2(IL + 1, IT + 1)] * xtr;
		b = b10 * (1.0 - xlr) + b11 * xlr;
	}

	if (symmetry == Symmetry::dipole_x) { bfield[0] = b; }
	else if (symmetry == Symmetry::dipole_y) { bfield[1] = b; }
	else { bfield[2] = b; }

	rotate_field(bfield);
}


void GField_AsciiMapFactory::value_cylindrical(const double x[3], double* bfield) const {
	// Axis 0 = transverse (radial), axis 1 = longitudinal.
	double LC = 0.0, TC = 0.0, phi = 0.0;
	if (symmetry == Symmetry::cyl_z) {
		LC = x[2]; TC = std::sqrt(x[0] * x[0] + x[1] * x[1]); phi = G4ThreeVector(x[0], x[1], x[2]).phi();
	}
	else if (symmetry == Symmetry::cyl_x) {
		LC = x[0]; TC = std::sqrt(x[1] * x[1] + x[2] * x[2]); phi = G4ThreeVector(x[1], x[2], x[0]).phi();
	}
	else /* cyl_y */ {
		LC = x[1]; TC = std::sqrt(x[0] * x[0] + x[2] * x[2]); phi = G4ThreeVector(x[2], x[0], x[1]).phi();
	}

	if (TC < startMap[0] || LC < startMap[1]) { return; }
	unsigned IT = static_cast<unsigned>(std::floor((TC - startMap[0]) / cellSize[0]));
	unsigned IL = static_cast<unsigned>(std::floor((LC - startMap[1]) / cellSize[1]));
	if (IT >= np[0] - 1 || IL >= np[1] - 1) { return; }

	double b1 = 0.0, b2 = 0.0;
	if (!linear) {
		if (std::fabs(startMap[0] + IT * cellSize[0] - TC) > std::fabs(startMap[0] + (IT + 1) * cellSize[0] - TC)) IT++;
		if (std::fabs(startMap[1] + IL * cellSize[1] - LC) > std::fabs(startMap[1] + (IL + 1) * cellSize[1] - LC)) IL++;
		b1 = B1[idx2(IT, IL)];
		b2 = B2[idx2(IT, IL)];
	}
	else {
		const double xtr = (TC - (startMap[0] + IT * cellSize[0])) / cellSize[0];
		const double xlr = (LC - (startMap[1] + IL * cellSize[1])) / cellSize[1];
		const double b10 = B1[idx2(IT, IL)] * (1.0 - xtr) + B1[idx2(IT + 1, IL)] * xtr;
		const double b11 = B1[idx2(IT, IL + 1)] * (1.0 - xtr) + B1[idx2(IT + 1, IL + 1)] * xtr;
		b1 = b10 * (1.0 - xlr) + b11 * xlr;
		const double b20 = B2[idx2(IT, IL)] * (1.0 - xtr) + B2[idx2(IT + 1, IL)] * xtr;
		const double b21 = B2[idx2(IT, IL + 1)] * (1.0 - xtr) + B2[idx2(IT + 1, IL + 1)] * xtr;
		b2 = b20 * (1.0 - xlr) + b21 * xlr;
	}

	if (symmetry == Symmetry::cyl_z) {
		bfield[0] = b1 * std::cos(phi); bfield[1] = b1 * std::sin(phi); bfield[2] = b2;
	}
	else if (symmetry == Symmetry::cyl_x) {
		bfield[0] = b2; bfield[1] = b1 * std::cos(phi); bfield[2] = b1 * std::sin(phi);
	}
	else /* cyl_y */ {
		bfield[1] = b2; bfield[0] = b1 * std::sin(phi); bfield[2] = b1 * std::cos(phi);
	}

	rotate_field(bfield);
}


void GField_AsciiMapFactory::value_phi_segmented(const double x[3], double* bfield) const {
	// Axis 0 = azimuthal, axis 1 = transverse, axis 2 = longitudinal. Fields are stored in the local
	// (first-segment) frame and rotated back to the query phi.
	double aC = std::atan2(x[1], x[0]) * rad; // phi
	if (aC < 0) { aC += 360 * deg; }
	const double tC = std::sqrt(x[0] * x[0] + x[1] * x[1]); // R
	const double lC = x[2];                                 // Z

	// Fold into the first 60-degree segment, keeping the rotation back to the lab.
	double aLC = aC;
	while (aLC / deg > 30) { aLC -= 60 * deg; }
	const double dphi = aC - aLC;
	const double aaLC = std::fabs(aLC);
	const int    sign = (aLC >= 0 ? 1 : -1);

	if (aC < startMap[0] || tC < startMap[1] || lC < startMap[2]) { return; }
	unsigned aI = static_cast<unsigned>(std::floor((aaLC - startMap[0]) / cellSize[0]));
	unsigned tI = static_cast<unsigned>(std::floor((tC - startMap[1]) / cellSize[1]));
	unsigned lI = static_cast<unsigned>(std::floor((lC - startMap[2]) / cellSize[2]));
	if (aI >= np[0] - 1 || tI >= np[1] - 1 || lI >= np[2] - 1) { return; }

	double mfield[3] = {0.0, 0.0, 0.0};
	if (!linear) {
		if (std::fabs(startMap[0] + aI * cellSize[0] - aaLC) > std::fabs(startMap[0] + (aI + 1) * cellSize[0] - aaLC)) aI++;
		if (std::fabs(startMap[1] + tI * cellSize[1] - tC) > std::fabs(startMap[1] + (tI + 1) * cellSize[1] - tC)) tI++;
		if (std::fabs(startMap[2] + lI * cellSize[2] - lC) > std::fabs(startMap[2] + (lI + 1) * cellSize[2] - lC)) lI++;
		mfield[0] = B1[idx3(aI, tI, lI)];
		mfield[1] = B2[idx3(aI, tI, lI)];
		mfield[2] = B3[idx3(aI, tI, lI)];
	}
	else {
		const double xaz = (aaLC - (startMap[0] + aI * cellSize[0])) / cellSize[0];
		const double xtr = (tC - (startMap[1] + tI * cellSize[1])) / cellSize[1];
		const double xlr = (lC - (startMap[2] + lI * cellSize[2])) / cellSize[2];

		const std::vector<float>* comps[3] = {&B1, &B2, &B3};
		for (int k = 0; k < 3; ++k) {
			const std::vector<float>& B = *comps[k];
			const double b00 = B[idx3(aI, tI, lI)] * (1 - xaz) + B[idx3(aI + 1, tI, lI)] * xaz;
			const double b01 = B[idx3(aI, tI, lI + 1)] * (1 - xaz) + B[idx3(aI + 1, tI, lI + 1)] * xaz;
			const double b10 = B[idx3(aI, tI + 1, lI)] * (1 - xaz) + B[idx3(aI + 1, tI + 1, lI)] * xaz;
			const double b11 = B[idx3(aI, tI + 1, lI + 1)] * (1 - xaz) + B[idx3(aI + 1, tI + 1, lI + 1)] * xaz;
			const double b0  = b00 * (1 - xtr) + b10 * xtr;
			const double b1  = b01 * (1 - xtr) + b11 * xtr;
			mfield[k]        = b0 * (1 - xlr) + b1 * xlr;
		}
	}

	// Rotate the local field back to the query azimuth.
	bfield[0] = sign * mfield[0] * std::cos(dphi / rad) - mfield[1] * std::sin(dphi / rad);
	bfield[1] = sign * mfield[0] * std::sin(dphi / rad) + mfield[1] * std::cos(dphi / rad);
	bfield[2] = sign * mfield[2];

	rotate_field(bfield);
}


void GField_AsciiMapFactory::value_cartesian3d(const double x[3], double* bfield) const {
	// Axis 0 = X, axis 1 = Y, axis 2 = Z.
	double XX = x[0], YY = x[1], ZZ = x[2];

	if (symmetry == Symmetry::cartesian_3d_quadrant) {
		// Fold the query point into the stored first quadrant (x>=0, y>=0).
		if (x[0] >= 0 && x[1] >= 0) { XX = x[0]; YY = x[1]; }
		else if (x[0] >= 0 && x[1] < 0) { XX = -x[1]; YY = x[0]; }
		else if (x[0] < 0 && x[1] < 0) { XX = -x[0]; YY = -x[1]; }
		else /* x[0] < 0 && x[1] >= 0 */ { XX = x[1]; YY = -x[0]; }
		ZZ = x[2];
		if (XX < 0 || YY < 0) { return; }
	}

	if (XX < startMap[0] || YY < startMap[1] || ZZ < startMap[2]) { return; }
	if (XX >= endMap[0] || YY >= endMap[1] || ZZ >= endMap[2]) { return; }
	const unsigned IXX = static_cast<unsigned>(std::floor((XX - startMap[0]) / cellSize[0]));
	const unsigned IYY = static_cast<unsigned>(std::floor((YY - startMap[1]) / cellSize[1]));
	const unsigned IZZ = static_cast<unsigned>(std::floor((ZZ - startMap[2]) / cellSize[2]));

	double B[3] = {0.0, 0.0, 0.0};
	if (!linear) {
		unsigned ix = IXX, iy = IYY, iz = IZZ;
		if (std::fabs(startMap[0] + ix * cellSize[0] - XX) > std::fabs(startMap[0] + (ix + 1) * cellSize[0] - XX)) ix++;
		if (std::fabs(startMap[1] + iy * cellSize[1] - YY) > std::fabs(startMap[1] + (iy + 1) * cellSize[1] - YY)) iy++;
		if (std::fabs(startMap[2] + iz * cellSize[2] - ZZ) > std::fabs(startMap[2] + (iz + 1) * cellSize[2] - ZZ)) iz++;
		B[0] = B1[idx3(ix, iy, iz)];
		B[1] = B2[idx3(ix, iy, iz)];
		B[2] = B3[idx3(ix, iy, iz)];
	}
	else {
		const double Xd = (XX - (startMap[0] + IXX * cellSize[0])) / cellSize[0];
		const double Yd = (YY - (startMap[1] + IYY * cellSize[1])) / cellSize[1];
		const double Zd = (ZZ - (startMap[2] + IZZ * cellSize[2])) / cellSize[2];

		const std::vector<float>* comps[3] = {&B1, &B2, &B3};
		for (int k = 0; k < 3; ++k) {
			const std::vector<float>& Bk = *comps[k];
			const double c00 = Bk[idx3(IXX, IYY, IZZ)] * (1 - Xd) + Bk[idx3(IXX + 1, IYY, IZZ)] * Xd;
			const double c01 = Bk[idx3(IXX, IYY, IZZ + 1)] * (1 - Xd) + Bk[idx3(IXX + 1, IYY, IZZ + 1)] * Xd;
			const double c10 = Bk[idx3(IXX, IYY + 1, IZZ)] * (1 - Xd) + Bk[idx3(IXX + 1, IYY + 1, IZZ)] * Xd;
			const double c11 = Bk[idx3(IXX, IYY + 1, IZZ + 1)] * (1 - Xd) + Bk[idx3(IXX + 1, IYY + 1, IZZ + 1)] * Xd;
			const double c0  = c00 * (1 - Yd) + c10 * Yd;
			const double c1  = c01 * (1 - Yd) + c11 * Yd;
			B[k]             = c0 * (1 - Zd) + c1 * Zd;
		}
	}

	if (symmetry == Symmetry::cartesian_3d_quadrant) {
		// Mirror the field components back to the query quadrant.
		if (x[0] >= 0 && x[1] >= 0) { bfield[0] = B[0]; bfield[1] = B[1]; }
		else if (x[0] >= 0 && x[1] < 0) { bfield[0] = B[1]; bfield[1] = -B[0]; }
		else if (x[0] < 0 && x[1] < 0) { bfield[0] = -B[0]; bfield[1] = -B[1]; }
		else /* x[0] < 0 && x[1] >= 0 */ { bfield[0] = -B[1]; bfield[1] = B[0]; }
		bfield[2] = B[2];
	}
	else {
		bfield[0] = B[0]; bfield[1] = B[1]; bfield[2] = B[2];
	}

	rotate_field(bfield);
}


void GField_AsciiMapFactory::rotate_field(double* bfield) const {
	// Rotate the field axes, not the point: each rotation is the inverse of the point rotation.
	if (mapRotation[0] != 0) {
		const double yPrime =  bfield[1] * cosAlpha + bfield[2] * sinAlpha;
		const double zPrime = -bfield[1] * sinAlpha + bfield[2] * cosAlpha;
		bfield[1] = yPrime; bfield[2] = zPrime;
	}
	if (mapRotation[1] != 0) {
		const double xPrime = bfield[0] * cosBeta - bfield[2] * sinBeta;
		const double zPrime = bfield[0] * sinBeta + bfield[2] * cosBeta;
		bfield[0] = xPrime; bfield[2] = zPrime;
	}
	if (mapRotation[2] != 0) {
		const double xPrime =  bfield[0] * cosGamma + bfield[1] * sinGamma;
		const double yPrime = -bfield[0] * sinGamma + bfield[1] * cosGamma;
		bfield[0] = xPrime; bfield[1] = yPrime;
	}
}
