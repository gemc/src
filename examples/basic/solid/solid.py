#!/usr/bin/env python3
"""Solid gallery: build any Geant4 solid from its name and parameter string.

Every volume here is created through the generic two-string interface

    gvolume.solid      = '<G4SolidType>'      # e.g. 'G4Torus'
    gvolume.parameters = '<p0*unit, p1*unit>'  # constructor order, with units

which is all the GEMC native geometry factory needs. Nothing here depends on a
per-solid ``make_*`` helper, so the same loop builds every solid GEMC supports.
The solid types and parameters live in ``solids_catalog.py``.

The geometry *variation* selects what is built:

  * ``all`` (and ``default``): every catalog solid, laid out on a grid;
  * ``<solid key>``          : just that one solid, centred at the origin.

Build one solid to an ascii database, for example::

    solid.py -f ascii -v torus

or the whole gallery::

    solid.py -f sqlite -v all
"""
from pygemc import autogeometry
from pygemc import GVolume
from solids_catalog import SOLIDS

cfg = autogeometry("examples", "solid")

# World large enough to contain the full grid with margin.
world = GVolume('root')
world.description = 'World'
world.make_box(600, 600, 300)
world.material = 'G4_AIR'
world.color = '889922'
world.style = 0
world.opacity = 0.1
world.publish(cfg)


def build_solid(name, solid_type, parameters, position=(0, 0, 0)):
    """Publish one volume built straight from its solid type and parameter string."""
    gvolume = GVolume(name)
    gvolume.description = solid_type
    gvolume.solid = solid_type
    gvolume.parameters = parameters
    gvolume.material = 'G4_WATER'
    gvolume.color = '3399ff'
    gvolume.set_position(*position)
    gvolume.publish(cfg)


variation = cfg.variation

if variation in ('all', 'default'):
    # Lay the solids out on a grid in the XY plane so none overlap.
    ncols = 6
    spacing = 180
    keys = list(SOLIDS)
    nrows = (len(keys) + ncols - 1) // ncols
    for i, key in enumerate(keys):
        col, row = i % ncols, i // ncols
        x = (col - (ncols - 1) / 2) * spacing
        y = (row - (nrows - 1) / 2) * spacing
        solid_type, parameters = SOLIDS[key]
        build_solid(key, solid_type, parameters, (x, y, 0))
elif variation in SOLIDS:
    solid_type, parameters = SOLIDS[variation]
    build_solid(variation, solid_type, parameters)
else:
    raise SystemExit(
        f"Unknown variation '{variation}'. Use 'all' or one of: {', '.join(SOLIDS)}")
