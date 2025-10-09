from g4_units import convert_list
import re
import numpy as np


def get_center(gvolume) -> tuple:
	raw = gvolume.position
	tokens = [t.strip() for t in raw.split(',') if t.strip()]
	return convert_list(tokens)


def get_dimensions(gvolume) -> tuple:
	raw = gvolume.parameters
	# split strictly on commas and trim; keep ALL tokens
	tokens = [t.strip() for t in raw.split(',') if t.strip()]
	return convert_list(tokens)


def pyvista_color(color_str: str, default_opacity: float = 1.0):
	"""
	Convert 'RRGGBB' or 'RRGGBB[T]' (T in 0..5; 5 = fully transparent) to:
	  ( (r,g,b) in 0..1, opacity in 0..1 )

	Accepts optional prefixes ('#', '0x') and ignores surrounding whitespace.
	"""
	s = color_str.strip().lower()
	if s.startswith('#'):
		s = s[1:]
	if s.startswith('0x'):
		s = s[2:]

	# Optional trailing transparency digit 0..5
	t_level = None
	if len(s) == 7 and s[-1].isdigit():
		t_level = int(s[-1])
		s = s[:-1]

	if len(s) != 6 or not re.fullmatch(r'[0-9a-f]{6}', s):
		raise ValueError(f"Invalid color string: {color_str!r} (expected RRGGBB or RRGGBB[T])")

	r = int(s[0:2], 16) / 255.0
	g = int(s[2:4], 16) / 255.0
	b = int(s[4:6], 16) / 255.0

	if t_level is None:
		opacity = float(default_opacity)
	else:
		# transparency level: 0 (opaque) … 5 (fully transparent)
		t_level = max(0, min(5, t_level))
		opacity = 1.0 - (t_level / 5.0)

	return (r, g, b), opacity


def render_volume(gvolume, gconfiguration):
	if gconfiguration.use_pyvista:
		rgb, alpha = pyvista_color(gvolume.color)
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
			f'Volume: {gvolume.name}, Solid: {gvolume.solid}, Center: {bcenter}, Pars: {pars}, Color: {rgb}, Alpha: {alpha}')

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

		print("before:", mesh.center)

		mesh = move_to_center(mesh, bcenter)
		print("after: ", mesh.center, " target:", bcenter)

		actor = gconfiguration.add_mesh(mesh, color=rgb, smooth_shading=True, opacity=alpha,
		                                style=mstyle, line_width=mlinewidth)
		actor.prop.ambient = 0.15  # a touch of ambient so faces aren’t pitch black


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
		x_length=pars[0]*2, y_length=pars[1]*2, z_length=pars[2]*2
	)
	return volume


def add_cylinder(pv, pars):
	res = 128  # resolution
	rmin = pars[0] * 0.5
	rmax = pars[1] * 0.5
	hz = pars[2] * 0.5
	phi_start = pars[3]
	dphi = pars[4]

	print(f'cylinder: rmin={rmin}, rmax={rmax}, hz={hz}, phi_start={phi_start}, dphi={dphi}')
	outer = pv.Cylinder(radius=rmax, height=2 * hz, resolution=res, direction=(0, 0, 1))
	if rmin > 0:
		inner = pv.Cylinder(radius=rmin, height=2 * hz, resolution=res, direction=(0, 0, 1))
		tub = outer.boolean_difference(inner.triangulate().clean())
	else:
		tub = outer
	if dphi < 360.0:
		# clip with two planes to realize a phi segment
		tub = tub.clip(normal=[0, 0, 1], origin=[0, 0, 0], invert=False)  # no-op, just example

		# Build two radial clip planes based on phi_start and phi_start+dphi:
		def radial_plane(phi_deg):
			a = np.radians(phi_deg)
			n = [np.cos(a), np.sin(a), 0.0]
			return n

		tub = tub.clip(normal=radial_plane(phi_start), origin=[0, 0, 0])
		tub = tub.clip(normal=[-c for c in radial_plane(phi_start + dphi)], origin=[0, 0, 0])
	return tub.triangulate().clean()


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
