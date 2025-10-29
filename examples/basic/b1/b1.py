#!/usr/bin/env python3

# gemc api
from gconfiguration import autogeometry

# build_geometry is in geometry.py
from geometry import build_geometry

cfg = autogeometry("examples", "b1")
build_geometry(cfg)

