#!/usr/bin/env python3

# gemc api:
from gconfiguration import autogeometry

# build_geometry and define_materials are in geometry.py, materials.py
from geometry import build_geometry
from materials import define_materials

cfg = autogeometry("examples", "b3")

build_geometry(cfg)
define_materials(cfg)

