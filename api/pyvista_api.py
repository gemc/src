from g4_units import convert_list
import numpy as np


def euler_matrix_zyx(deg_rx, deg_ry, deg_rz):
	"""
	Build a 3x3 rotation matrix from intrinsic ZYX Euler angles.
	Convention:
	  - rz about Z
	  - ry about Y
	  - rx about X
	Angles are in degrees.
	"""
	rx = np.deg2rad(deg_rx)
	ry = np.deg2rad(deg_ry)
	rz = np.deg2rad(deg_rz)

	cz, sz = np.cos(rz), np.sin(rz)
	cy, sy = np.cos(ry), np.sin(ry)
	cx, sx = np.cos(rx), np.sin(rx)

	# Rz
	Rz = np.array([
		[cz, -sz, 0.0],
		[sz, cz, 0.0],
		[0.0, 0.0, 1.0],
	])

	# Ry
	Ry = np.array([
		[cy, 0.0, sy],
		[0.0, 1.0, 0.0],
		[-sy, 0.0, cy],
	])

	# Rx
	Rx = np.array([
		[1.0, 0.0, 0.0],
		[0.0, cx, -sx],
		[0.0, sx, cx],
	])

	# intrinsic ZYX means: v_local -> Rx -> Ry -> Rz
	# matrix multiply in that order: R = Rz @ Ry @ Rx
	return Rx @ Ry @ Rz


class GMesh:
	def __init__(
			self,
			name,
			mesh,
			mother=None,
			material=None,
			mfield=None,
			color="white",
			position=(0.0, 0.0, 0.0),
			rotation=(0.0, 0.0, 0.0),
			opacity=1.0,
	):
		"""
		name:       string ID (must be unique so we can look it up in the dict)
		mesh:       pyvista mesh in its own local coordinates
		mother:     string name of parent GMesh, or None
		material:   e.g. "G4_Aluminum"
		mfield:     e.g. "solenoid_field"
		color:      display color
		position:   local translation (x,y,z) relative to mother
		rotation:   local rotation (rx, ry, rz) in degrees, intrinsic ZYX
					i.e. rotate around X, then Y, then Z at this node
		"""
		self.name = name
		self.mesh = mesh
		self.mother = mother
		self.material = material
		self.mfield = mfield
		self.color = color
		self.opacity = float(opacity)

		self.position = np.array(position, dtype=float)
		self.rotation = np.array(rotation, dtype=float)  # (rx, ry, rz) degrees

		# caches
		self._world_position = None  # np.array([x,y,z])
		self._world_rotation = None  # 3x3 np.array

	def compute_world_transform(self, lookup):
		"""
		Returns (R_world, T_world)
		where:
		  R_world is a 3x3 rotation matrix taking *this mesh's local coords*
		  into world coords.
		  T_world is a 3-vector translation in world coords.
		Recursively accumulates parent's transform.
		Caches results.
		"""

		if (self._world_rotation is not None) and (self._world_position is not None):
			return self._world_rotation, self._world_position

		# local rotation and translation of THIS node
		R_local = euler_matrix_zyx(*self.rotation)
		T_local = self.position

		if self.mother is None:
			# root: world = local
			R_world = R_local
			T_world = T_local
		else:
			parent = lookup[self.mother]
			R_parent, T_parent = parent.compute_world_transform(lookup)

			# child world rotation = R_parent * R_local
			R_world = R_parent @ R_local

			# child world translation = T_parent + R_parent * T_local
			# (because child's local offset is expressed in parent's frame,
			#  which may itself be rotated)
			T_world = T_parent + (R_parent @ T_local)

		# cache
		self._world_rotation = R_world
		self._world_position = T_world

		return R_world, T_world

	def world_mesh(self, lookup):
		"""
		Build and return a transformed copy of self.mesh, in world coordinates.
		Applies rotation first, then translation.
		"""
		R_world, T_world = self.compute_world_transform(lookup)

		# copy so we don't mutate original
		world_copy = self.mesh.copy()

		# apply rotation: rotate points around origin using R_world
		pts = world_copy.points.copy()  # (N,3)
		pts = pts @ R_world.T  # apply rotation
		# note: we do pts @ R^T because pts are row vectors

		# apply translation
		pts = pts + T_world

		# update mesh points
		world_copy.points = pts

		return world_copy


def get_center(gvolume) -> tuple:
	raw = gvolume.position
	tokens = [t.strip() for t in raw.split(',') if t.strip()]
	return convert_list(tokens)


def get_dimensions(gvolume) -> tuple:
	raw = gvolume.parameters
	# split strictly on commas and trim; keep ALL tokens
	tokens = [t.strip() for t in raw.split(',') if t.strip()]
	return convert_list(tokens)


def render_volume(gvolume, gconfiguration):
	if gconfiguration.use_pyvista:
		metallic = False
		pcolor = gvolume.color
		rgb = gvolume.gcolor  # already converted to 'RRGGBB'
		alpha = gvolume.opacity
		# rgb = pyvista_gcolor_to_pcolor(gcolor)
		# print(f"Volume {gvolume.name} color: {pcolor} / gcolor: {gcolor}")
		if pcolor != '778899':  # hardcoded from default
			# if pcolor is 2 strings, check if the first string is 'metallic'
			if ',' in pcolor:
				parts = pcolor.split(',')
				if len(parts) == 2:
					if parts[0].lower() == 'metallic':
						metallic = True
					rgb = parts[1]
			else:
				rgb = pcolor
		pv = gconfiguration.pv
		pars = get_dimensions(gvolume)
		bcenter = get_center(gvolume)
		mstyle = "surface" if gvolume.style == 1 else "wireframe"
		mlinewidth = 1.0 if gvolume.style == 1 else 10
		if gvolume.visible == 0:
			alpha = 0.05  # nearly invisible
			mlinewidth = 1.0
			mstyle = "wireframe"

		# print pars
		print(
			f'Volume: {gvolume.name}, Solid: {gvolume.solid}, Center: {bcenter}, '
			f'Pars: {pars}, Color: {rgb}, Alpha: {alpha}')

		mesh = None

		if gvolume.solid == 'G4Box':
			mesh = add_box(pv, pars)
		elif gvolume.solid == 'G4Cons':
			mesh = add_cons(pv, pars)
		elif gvolume.solid == 'G4Tubs':
			mesh = add_cylinder(pv, pars)
		elif gvolume.solid == 'G4Trd':
			mesh = add_trapezoid(pv, pars)
		if mesh is None:
			return

		mesh = move_to_center(mesh, bcenter)

		actor = gconfiguration.add_mesh(mesh, color=rgb, smooth_shading=True, opacity=alpha,
		                                style=mstyle, line_width=mlinewidth)
		actor.prop.ambient = 0.15  # a touch of ambient so faces aren’t pitch black
		if metallic:
			actor.prop.interpolation = "pbr"
			actor.prop.metallic = 0.4
			actor.prop.roughness = 0.4


def move_to_center(mesh, target_center):
	"""Translate mesh so its center becomes target_center."""
	target = np.asarray(target_center, dtype=float)
	curr = np.asarray(mesh.center, dtype=float)
	delta = target - curr
	out = mesh.copy()
	out.translate(tuple(delta), inplace=True)
	return out


def add_box(pv, pars) -> None:
	volume = pv.Cube(
		x_length=pars[0] * 2, y_length=pars[1] * 2, z_length=pars[2] * 2
	)
	return volume


def add_cylinder(pv, pars):
	"""
	pars = (rmin, rmax, hz, phi_start_deg, dphi_deg)
	Builds a watertight G4Tubs without boolean ops.
	"""
	rmin, rmax, hz, phi_start, dphi = map(float, pars)
	if rmax <= 0 or hz <= 0:
		raise ValueError(f"Invalid cylinder sizes: rmax={rmax}, hz={hz}")

	# angular resolution: denser for small sectors
	res = max(32, int(256 * (dphi / 360.0 if dphi < 360.0 else 1.0)))

	def polydata_from_pts_faces(pts, faces_idx):
		poly = pv.PolyData()
		poly.points = np.array(pts, dtype=float)
		# faces_idx like [n, i0,i1,..., in-1]
		poly.faces = np.array(faces_idx, dtype=np.int64)
		return poly

	if rmin <= 0.0:
		# SOLID cylinder: include the axis so the revolution fills the volume
		# Quad in X–Z plane (Y=0): (rmax,-hz)->(rmax,+hz)->(0,+hz)->(0,-hz)
		pts = [
			[rmax, 0.0, -hz],
			[rmax, 0.0, +hz],
			[0.0, 0.0, +hz],
			[0.0, 0.0, -hz],
		]
		faces = [4, 0, 1, 2, 3]  # one quad
		profile = polydata_from_pts_faces(pts, faces)
	else:
		# HOLLOW tube: a rectangular ring in (r,z): (rmin,-hz)->(rmax,-hz)->(rmax,+hz)->(rmin,+hz)
		# This revolved surface is closed in 360° and remains watertight for partial φ with capping
		pts = [
			[rmin, 0.0, -hz],  # 0
			[rmax, 0.0, -hz],  # 1
			[rmax, 0.0, +hz],  # 2
			[rmin, 0.0, +hz],  # 3
		]
		faces = [4, 0, 1, 2, 3]
		profile = polydata_from_pts_faces(pts, faces)

	# Revolve only the requested angle; capping=True closes ends (and radial faces for φ<360)
	tube = profile.extrude_rotate(angle=dphi, resolution=res, capping=True)

	# Align φ start
	if dphi < 360.0 or abs(phi_start) > 1e-12:
		tube = tube.rotate_z(phi_start, inplace=False)

	return tube.triangulate().clean()


def add_cons(pv, pars):
	"""
	Build a G4Cons-like solid.

	Parameters (numerical; already in your scene units):
	  pars[0] = rin1     # inner radius at z = -length
	  pars[1] = rout1    # outer radius at z = -length
	  pars[2] = rin2     # inner radius at z = +length
	  pars[3] = rout2    # outer radius at z = +length
	  pars[4] = length   # HALF length in z (i.e., dz)
	  pars[5] = phi_start (degrees)
	  pars[6] = phi_total (degrees)

	bcenter: (x, y, z) center in world coords.

	Returns: closed PolyData surface suitable for rendering/CSG.
	"""
	# --- unpack
	rin1 = pars[0]
	rout1 = pars[1]
	rin2 = pars[2]
	rout2 = pars[3]
	hz = pars[4]
	phi_start = pars[5]
	phi_total = pars[6]

	res = 128  # resolution

	def solid_to_axis(r0, z0, r1, z1):
		"""Make a quad polygon in X–Z plane (Y=0) that includes the axis."""
		pts = np.array([
			[r0, 0.0, z0],  # outer @ z0
			[r1, 0.0, z1],  # outer @ z1
			[0.0, 0.0, z1],  # axis @ z1
			[0.0, 0.0, z0],  # axis @ z0
		], dtype=float)
		poly = pv.PolyData()
		poly.points = pts
		# one polygon with 4 verts: [n, i0, i1, i2, i3]
		poly.faces = np.array([4, 0, 1, 2, 3], dtype=np.int64)
		return poly

	# Build a solid outer frustum-to-axis and revolve only phi_total
	outer_profile = solid_to_axis(rout1, -hz, rout2, +hz)
	outer = outer_profile.extrude_rotate(angle=phi_total, resolution=res, capping=True)

	# If inner radii present, build inner solid-to-axis and subtract
	have_inner = (rin1 > 0.0) or (rin2 > 0.0)
	if have_inner:
		inner_profile = solid_to_axis(rin1, -hz, rin2, +hz)
		inner = inner_profile.extrude_rotate(angle=phi_total, resolution=res, capping=True)
		cons = outer.triangulate().clean().boolean_difference(inner.triangulate().clean())
	else:
		cons = outer

	# Rotate sector start and translate to center
	if phi_total < 360.0 or abs(phi_start) > 1e-12:
		cons = cons.rotate_z(phi_start, inplace=False)

	# Final tidy
	cons = cons.triangulate().clean()

	return cons


def add_trapezoid(pv, pars) -> None:
	"""
	Build a G4Trd in PyVista.

	Geant4 parameter order (half-lengths):
	  pars[0] = dx1  # half-length X at z = -dz
	  pars[1] = dx2  # half-length X at z = +dz
	  pars[2] = dy1  # half-length Y at z = -dz
	  pars[3] = dy2  # half-length Y at z = +dz
	  pars[4] = dz   # half-length in Z

	bcenter: (x,y,z) where the solid’s local origin (0,0,0) should land.
	Returns: closed PolyData.
	"""
	dx1, dx2, dy1, dy2, dz = map(float, pars[:5])

	# 	dx1 *= 0.5;
	# 	dx2 *= 0.5;
	# 	dy1 *= 0.5;
	# 	dy2 *= 0.5;
	# 	dz *= 0.5

	z0, z1 = -dz, +dz

	# Eight vertices: bottom (-z) then top (+z)
	# Order each face CCW when viewed from outside.
	pts = np.array([
		[-dx1, -dy1, z0],  # 0 bottom
		[+dx1, -dy1, z0],  # 1
		[+dx1, +dy1, z0],  # 2
		[-dx1, +dy1, z0],  # 3
		[-dx2, -dy2, z1],  # 4 top
		[+dx2, -dy2, z1],  # 5
		[+dx2, +dy2, z1],  # 6
		[-dx2, +dy2, z1],  # 7
	], dtype=float)

	# Six quad faces (bottom, top, and 4 sides)
	faces = np.array([
		4, 0, 1, 2, 3,  # bottom (-z)
		4, 4, 5, 6, 7,  # top (+z)
		4, 0, 1, 5, 4,  # -Y side
		4, 1, 2, 6, 5,  # +X side
		4, 2, 3, 7, 6,  # +Y side
		4, 3, 0, 4, 7,  # -X side
	], dtype=np.int64)

	trd = pv.PolyData(pts, faces)
	# (Optional) robustness: ensure triangulated & watertight
	trd = trd.triangulate().clean()

	return trd


def _is_box_like(mesh):
	"""
	Heuristic for pv.Cube():
	- PolyData
	- typically 8 points, axis-aligned bounds
	"""
	import pyvista as pv
	if not isinstance(mesh, pv.PolyData):
		return False
	# pv.Cube usually has 8 points. We keep it simple.
	return mesh.n_points == 8


def _is_cylinder_like(mesh):
	"""
	Heuristic for pv.Cylinder():
	- PolyData
	- many points (circular cross-section tessellated)
	- XY extents ~same
	"""
	import pyvista as pv
	if not isinstance(mesh, pv.PolyData):
		return False
	if mesh.n_points < 20:
		return False

	xmin, xmax, ymin, ymax, zmin, zmax = mesh.bounds
	rx = 0.5 * (xmax - xmin)
	ry = 0.5 * (ymax - ymin)
	if rx == 0:
		return False
	ratio = ry / rx
	return 0.8 <= ratio <= 1.25  # "round-ish" in xy


def _is_sphere_like(mesh):
	"""
	Heuristic for pv.Sphere():
	- PolyData
	- many points
	- extents in x,y,z similar
	"""
	import pyvista as pv
	if not isinstance(mesh, pv.PolyData):
		return False
	if mesh.n_points < 30:
		return False

	xmin, xmax, ymin, ymax, zmin, zmax = mesh.bounds
	dx = xmax - xmin
	dy = ymax - ymin
	dz = zmax - zmin
	if dx == 0 or dy == 0:
		return False
	xy_ratio = dy / dx
	yz_ratio = dz / dy if dy != 0 else 0.0

	return (0.8 <= xy_ratio <= 1.25) and (0.8 <= yz_ratio <= 1.25)


def gmesh_to_geant4_solid_and_params(gm, length_unit='mm', angle_unit='deg'):
	"""
	Given a GMesh (which holds gm.mesh: a PyVista mesh),
	return (solid_name, parameters_string) suitable for GVolume.

	Supported so far:
	  - pv.Cube      -> G4Box
	  - pv.Cylinder  -> G4Tubs  (full 360°, solid)
	  - pv.Sphere    -> G4Sphere (full sphere)
	"""

	mesh = gm.mesh
	xmin, xmax, ymin, ymax, zmin, zmax = mesh.bounds

	# Box (pv.Cube -> G4Box)
	if _is_box_like(mesh):
		# Geant4 G4Box takes HALF-lengths along x,y,z
		px = 0.5 * (xmax - xmin)
		py = 0.5 * (ymax - ymin)
		pz = 0.5 * (zmax - zmin)

		param_string = f"{px}*{length_unit}, {py}*{length_unit}, {pz}*{length_unit}"
		return "G4Box", param_string

	# Cylinder (pv.Cylinder -> G4Tubs)
	# We'll assume direction=(0,0,1) in local coords,
	# so height is along Z, radius in XY.
	if _is_cylinder_like(mesh):
		rx = 0.5 * (xmax - xmin)
		ry = 0.5 * (ymax - ymin)
		rmax = 0.5 * ((xmax - xmin) + (ymax - ymin)) / 2.0
		rmin = 0.0  # solid tube
		half_z = 0.5 * (zmax - zmin)

		sphi = 0.0
		dphi = 360.0

		param_string = (
			f"{rmin}*{length_unit}, "
			f"{rmax}*{length_unit}, "
			f"{half_z}*{length_unit}, "
			f"{sphi}*{angle_unit}, "
			f"{dphi}*{angle_unit}"
		)
		return "G4Tubs", param_string

	# Sphere (pv.Sphere -> G4Sphere)
	if _is_sphere_like(mesh):
		rx = 0.5 * (xmax - xmin)
		ry = 0.5 * (ymax - ymin)
		rz = 0.5 * (zmax - zmin)
		rmax = (rx + ry + rz) / 3.0  # average radius
		rmin = 0.0

		sphi = 0.0
		dphi = 360.0
		stheta = 0.0
		dtheta = 180.0

		param_string = (
			f"{rmin}*{length_unit}, "
			f"{rmax}*{length_unit}, "
			f"{sphi}*{angle_unit}, "
			f"{dphi}*{angle_unit}, "
			f"{stheta}*{angle_unit}, "
			f"{dtheta}*{angle_unit}"
		)
		return "G4Sphere", param_string

	# Not recognized yet (cones/polycones/etc.)
	return None, None


def pvmeshes_from_gmeshes(gmeshes):
	# build lookup dict for hierarchy resolution
	shape_dict = {gm.name: gm for gm in gmeshes}

	pymeshes = []

	# add world-space meshes
	for gm in gmeshes:
		world_poly = gm.world_mesh(shape_dict)
		pymeshes.append(
			(world_poly, gm.color, gm.opacity)
		)

	return pymeshes


def set_yz_view_x_into_screen(p, distance=10.0):
	"""
	Arrange the camera so:
	  - +Z points to the right on screen
	  - +X points into the screen (depth)
	  - We're looking along -X toward the origin

	Note:
	  This choice forces screen-up to be world -Y.
	  In other words, +Y will appear downward.
	"""
	eye = (-distance, 0.0, 0.0)  # camera sitting on -X
	focus = (0.0, 0.0, 0.0)  # look at origin -> looking along -X
	view_up = (0.0, 1.0, 0.0)  # flipped so Z is to the right

	p.camera_position = [eye, focus, view_up]
