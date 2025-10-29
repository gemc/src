#!/usr/bin/env python3

# gemc api:
from gconfiguration import autogeometry
from pyvista_api import GMesh
from gvolume import GVolume

# pyvista objects
from pyvista_basic_shapes import make_basic_shapes

cfg = autogeometry("examples", "pyvista")

for gm in make_basic_shapes():
	gv = GVolume.from_gmesh(gm)
	gv.publish(cfg)
