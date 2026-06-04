#!/usr/bin/env python3
# Creates the simple_flux system used by test_reload_sequence.
# World half-size = 150 mm so the proton gun at z = -100 mm is inside.
from pygemc import autogeometry, GVolume

cfg = autogeometry("examples", "simple_flux")

gvolume = GVolume('root')
gvolume.description = 'World'
gvolume.make_box(150.0, 150.0, 150.0)
gvolume.material = 'G4_AIR'
gvolume.color = 'ghostwhite'
gvolume.style = 0
gvolume.publish(cfg)

gvolume = GVolume('FluxPlane')
gvolume.mother = 'root'
gvolume.description = 'Flux Scoring Plane'
gvolume.make_box(120.0, 120.0, 0.5)
gvolume.material = 'G4_AIR'
gvolume.color = 'FAFAD2'
gvolume.set_position(0, 0, 50)
gvolume.digitization = 'flux'
gvolume.set_identifier('flux_plane', 1)
gvolume.publish(cfg)
