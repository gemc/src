#!/usr/bin/env python3

from pygemc import autogeometry
from pygemc import GVolume
from pyvista_basic_shapes import make_basic_shapes

cfg = autogeometry("examples", "pyvista")

for gm in make_basic_shapes():
    gv = GVolume.from_gmesh(gm)
    gv.publish(cfg)
