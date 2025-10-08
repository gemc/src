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

		# print pars
		print(
			f'Volume: {gvolume.name}, Solid: {gvolume.solid}, Center: {bcenter}, Pars: {pars}, Color: {rgb}, Alpha: {alpha}')

		mesh = None

		if gvolume.solid == 'G4Box':
			mesh = add_box(pv, pars, bcenter)
		elif gvolume.solid == 'G4Cons':
			pass
		elif gvolume.solid == 'G4Tubs':
			mesh = add_cylinder(pv, pars, bcenter)
		elif gvolume.solid == 'G4Trap8':
			pass
		else:
			print(f'\n Fatal error: {gvolume.solid} not supported yet')
			exit(1)

		if mesh is None:
			return
		actor = gconfiguration.add_mesh(mesh, color=rgb, smooth_shading=True, opacity=alpha, style=mstyle, line_width=mlinewidth)
		actor.prop.ambient = 0.15  # a touch of ambient so faces aren’t pitch black


def add_box(pv, pars, bcenter) -> None:
	volume = pv.Cube(
		center=bcenter,
		x_length=pars[0], y_length=pars[1], z_length=pars[2]
	)
	return volume


def add_cylinder(pv, pars, bcenter):
	res = 128  # resolution
	rmin = pars[0] / 2
	rmax = pars[1] / 2
	hz = pars[2] / 2
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
