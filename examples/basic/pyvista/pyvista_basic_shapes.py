#!/usr/bin/env python3

import pyvista as pv
from pyvista_api import GMesh, pvmeshes_from_gmeshes, set_yz_view_x_into_screen


def make_basic_shapes():
	gmeshes = []

	# cube is the root, shifted -1 in x and rotated a bit
	cube_mesh = pv.Cube(
		x_length=2.0,
		y_length=1.0,
		z_length=4.0,
	)

	# add material
	cube_gm = GMesh(
		name="cube",
		mesh=cube_mesh,
		material="G4_AIR",
		color="lightblue",
		opacity=0.4,
		position=(-1.0, 0.0, 0.0),  # translation in world frame
		rotation=(0.0, 0.0, 10.0),  # rotate cube 30° around Z
	)
	gmeshes.append(cube_gm)

	# cylinder is defined at its own local origin (0,0,0), unrotated locally
	cyl_mesh = pv.Cylinder(
		radius=0.5,
		height=1.0,
		resolution=64,
	)

	cyl_gm = GMesh(
		name="cylinder",
		mesh=cyl_mesh,
		mother="cube",  # inside
		material="G4_WATER",
		color="lightgreen",
		position=(0.0, 0.2, 0.0),  # 1 unit along mother's +Y
		rotation=(0.0, 10.0, 0.0),  # rotate 10° around mother's Y
	)
	gmeshes.append(cyl_gm)

	return gmeshes


if __name__ == "__main__":

	gmeshes = make_basic_shapes()
	pvmeshes = pvmeshes_from_gmeshes(gmeshes)

	p = pv.Plotter()

	# add world-space meshes
	for mesh, color, opacity in pvmeshes:
		p.add_mesh(mesh,
		           color=color,
		           opacity=opacity,
		           show_edges=True)

		# visual reference axes at world origin
		p.add_axes_at_origin(xlabel="X", ylabel="Y", zlabel="Z")
		p.show_axes()

	set_yz_view_x_into_screen(p, distance=10.0)

	p.show()

