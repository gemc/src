#!/usr/bin/env python3

import pyvista as pv
from pyvista_api import GMesh, pvmeshes_from_gmeshes, set_yz_view_x_into_screen


def make_basic_shapes():
    gmeshes = []

    # --- Box: world-like container, slightly transparent ---
    box_mesh = pv.Cube(x_length=4.0, y_length=4.0, z_length=6.0)
    box_gm = GMesh(
        name="box",
        mesh=box_mesh,
        material="G4_AIR",
        color="ghostwhite",
        opacity=0.15,
        position=(0.0, 0.0, 0.0),
        rotation=(0.0, 0.0, 0.0),
    )
    gmeshes.append(box_gm)

    # --- Cylinder: solid, inside the box ---
    cyl_mesh = pv.Cylinder(radius=0.6, height=2.0, resolution=64, direction=(0, 0, 1))
    cyl_gm = GMesh(
        name="cylinder",
        mesh=cyl_mesh,
        mother="box",
        material="G4_WATER",
        color="steelblue",
        opacity=1.0,
        position=(0.0, 0.0, -1.5),
        rotation=(0.0, 0.0, 0.0),
    )
    gmeshes.append(cyl_gm)

    # --- Sphere: inside the box, demonstrates G4Sphere ---
    sphere_mesh = pv.Sphere(radius=0.7, theta_resolution=32, phi_resolution=32)
    sphere_gm = GMesh(
        name="sphere",
        mesh=sphere_mesh,
        mother="box",
        material="G4_Pb",
        color="tomato",
        opacity=0.85,
        position=(0.0, 0.0, 1.2),
        rotation=(0.0, 0.0, 0.0),
    )
    gmeshes.append(sphere_gm)

    # --- Small rotated box: demonstrates rotation parameter ---
    rotbox_mesh = pv.Cube(x_length=1.2, y_length=0.4, z_length=1.2)
    rotbox_gm = GMesh(
        name="rotated_box",
        mesh=rotbox_mesh,
        mother="box",
        material="G4_Al",
        color="gold",
        opacity=0.9,
        position=(0.8, 0.8, 0.0),
        rotation=(0.0, 0.0, 35.0),  # 35 degrees around Z
    )
    gmeshes.append(rotbox_gm)

    return gmeshes


if __name__ == "__main__":
    gmeshes = make_basic_shapes()
    pvmeshes = pvmeshes_from_gmeshes(gmeshes)

    p = pv.Plotter()

    for mesh, color, opacity in pvmeshes:
        p.add_mesh(mesh, color=color, opacity=opacity, show_edges=True)

    p.add_axes_at_origin(xlabel="X", ylabel="Y", zlabel="Z")
    p.show_axes()
    set_yz_view_x_into_screen(p, distance=10.0)
    p.show()
