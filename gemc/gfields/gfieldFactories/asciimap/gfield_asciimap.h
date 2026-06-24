#pragma once

// gemc gfield framework
#include "gfield.h"

// c++
#include <array>
#include <string>
#include <vector>

/**
 * @defgroup gfield_asciimap_factory GField ASCII-map factory
 * @ingroup gfield_module
 *
 * \brief Generic ASCII field-map plugin (dipole, cylindrical, phi-segmented, cartesian 3D).
 *
 * This is the GEMC3 successor of the clas12Tags `asciiField`. The two responsibilities of the legacy
 * implementation are now split:
 *
 * - the **map definition** (symmetry, grid coordinates, units, interpolation, placement) lives in the
 *   YAML options, forwarded verbatim through the generic `gfields` node (the same mechanism used by the
 *   clas12-systems plugins);
 * - the **map file** holds only data rows (coordinate columns followed by field components). The legacy
 *   embedded `<mfield> ... </mfield>` XML header is gone.
 *
 * The plugin is selected with `type: asciimap`, which resolves to the shared library
 * `gfieldasciimapFactory`.
 */

/**
 * \brief Concrete \ref GField reading a magnetic field from an ASCII map and a YAML definition.
 * @ingroup gfield_asciimap_factory
 *
 * \par Supported symmetries
 * - `dipole-x`, `dipole-y`, `dipole-z`: 2D map of a single on-axis component, depending on a
 *   longitudinal and a transverse coordinate.
 * - `cylindrical-x`, `cylindrical-y`, `cylindrical-z`: 2D phi-symmetric map of a transverse (radial)
 *   and a longitudinal component.
 * - `phi-segmented`: 3D map in cylindrical coordinates (azimuthal, transverse, longitudinal) of three
 *   cartesian components, replicated over 60-degree segments.
 * - `cartesian_3D`, `cartesian_3D_quadrant`: 3D cartesian map of three cartesian components. The
 *   `_quadrant` variant stores only the first quadrant (x>=0, y>=0) and mirrors it.
 *
 * \par YAML parameters (all forwarded as scalar strings through the `gfields` node)
 * - `symmetry`   : one of the names above. Mandatory.
 * - `map`        : map file name. When it contains a `/` it is used as-is, otherwise it is looked up in
 *                  `dir`. Mandatory.
 * - `dir`        : directory holding the map. Optional; when empty the map is read from the `fields`
 *                  directory installed next to this plugin (`<plugin_dir>/../fields`).
 * - `coordinate1`, `coordinate2`, `coordinate3` : one grid axis each, as the compact string
 *                  `"name, npoints, min, max"`. `min`/`max` are Geant4-number expressions with units
 *                  (e.g. `0*m`, `3*m`); the unit there also sets the unit of the matching coordinate
 *                  column in the map file. `coordinate3` is required only for the 3D symmetries. The
 *                  column order in the map file follows `coordinate1`, `coordinate2`, `coordinate3`.
 * - `field_unit` : unit of the field columns in the map file (default `gauss`).
 * - `scale`      : dimensionless multiplier applied to every field value (default `1`).
 * - `interpolation` : `linear` (default) or `none` (nearest neighbour).
 * - `vx`,`vy`,`vz` : map origin subtracted from the query point before lookup (default `0`).
 * - `rx`,`ry`,`rz` : map rotation applied to the field vector (default `0*deg`).
 *
 * \par Coordinate names per symmetry
 * - dipole       : `longitudinal`, `transverse`
 * - cylindrical  : `transverse`, `longitudinal`
 * - phi-segmented: `azimuthal`, `transverse`, `longitudinal`
 * - cartesian    : `X`, `Y`, `Z`
 *
 * \par Implementation notes (improvements over the legacy asciiField)
 * - Field values are stored in contiguous `std::vector<float>` buffers addressed with precomputed
 *   strides, instead of the legacy `float**`/`float***` pointer pyramids; this keeps the hot
 *   \ref GetFieldValue loop cache friendly and removes manual `new`/`delete`.
 * - The symmetry is decoded once into an enum, so \ref GetFieldValue dispatches on an integer rather
 *   than comparing strings at every step.
 * - Rotation trigonometry is cached at load time.
 * - The map rows may be listed in any order: the grid index of each row is computed from its
 *   coordinate columns and validated against the YAML grid.
 */
class GField_AsciiMapFactory : public GField {

public:
	explicit GField_AsciiMapFactory(const std::shared_ptr<GOptions>& gopt) : GField(gopt) {}

	/// Compute the field at the lab-frame point `pos`, writing `{Bx,By,Bz}` (Geant4 units) into `bfield`.
	void GetFieldValue(const double pos[3], G4double* bfield) const override;

	/// Parse the YAML definition, build the grid and read the map file.
	void load_field_definitions(GFieldDefinition gfd) override;

private:
	/// Map symmetry, decoded once from the `symmetry` parameter.
	enum class Symmetry {
		dipole_x, dipole_y, dipole_z,
		cyl_x, cyl_y, cyl_z,
		phi_segmented,
		cartesian_3d, cartesian_3d_quadrant
	};

	Symmetry symmetry = Symmetry::dipole_z;
	int      ndim     = 2; ///< number of grid coordinates (2 or 3)
	int      ncomp    = 1; ///< number of field columns in the map file (1, 2 or 3)
	bool     linear   = true; ///< interpolation: true=linear, false=nearest neighbour

	// Grid geometry in canonical per-symmetry axis order (see class documentation).
	std::array<unsigned, 3> np       = {0, 0, 0};
	std::array<double, 3>   startMap  = {0.0, 0.0, 0.0};
	std::array<double, 3>   endMap    = {0.0, 0.0, 0.0};
	std::array<double, 3>   cellSize  = {1.0, 1.0, 1.0};

	// One map-file column per grid coordinate, in file-column order (coordinate1, coordinate2, ...).
	struct Column {
		int    axis       = 0;   ///< canonical axis this column feeds
		double unitFactor = 1.0; ///< multiplier converting the column value to Geant4 units
	};
	std::vector<Column> columns;

	// Field buffers (contiguous). B2/B3 stay empty for symmetries that do not use them.
	std::vector<float> B1;
	std::vector<float> B2;
	std::vector<float> B3;

	// Overall placement (lab frame). Origin in Geant4 length units, rotation in radians.
	double mapOrigin[3]   = {0.0, 0.0, 0.0};
	double mapRotation[3] = {0.0, 0.0, 0.0};
	double sinAlpha = 0.0, cosAlpha = 1.0; // about X
	double sinBeta  = 0.0, cosBeta  = 1.0; // about Y
	double sinGamma = 0.0, cosGamma = 1.0; // about Z

	// Flat indexing helpers (canonical axis order).
	inline std::size_t idx2(unsigned i0, unsigned i1) const { return static_cast<std::size_t>(i0) * np[1] + i1; }
	inline std::size_t idx3(unsigned i0, unsigned i1, unsigned i2) const {
		return (static_cast<std::size_t>(i0) * np[1] + i1) * np[2] + i2;
	}

	// Per-symmetry field evaluation (point already shifted by mapOrigin).
	void value_dipole(const double x[3], double* bfield) const;
	void value_cylindrical(const double x[3], double* bfield) const;
	void value_phi_segmented(const double x[3], double* bfield) const;
	void value_cartesian3d(const double x[3], double* bfield) const;

	// Rotate the field vector (not the point): each rotation is the inverse of the point rotation.
	void rotate_field(double* bfield) const;

	// Configuration helpers (the generic node only guarantees name/type).
	std::string param_string(const std::string& key, const std::string& dflt) const;
	double      param_g4number(const std::string& key, const std::string& dflt) const;

	// Map the canonical axis of a coordinate name for the current symmetry, or -1 if it does not belong.
	int axis_of_coordinate(const std::string& name) const;

	// Parse one "name, npoints, min, max" coordinate string into the grid arrays and a Column.
	void load_coordinate(const std::string& key);

	// Read the map file and fill the field buffers.
	void load_map_file();

	// Default field-map directory: <plugin_dir>/../fields (the `meson install` layout). No env var.
	std::string field_maps_directory() const;
};
