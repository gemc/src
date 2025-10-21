#!/usr/bin/env python3
from utils_api import autogeometry
from geometry_api import GVolume

configuration = autogeometry("examples", "simple_flux")

world_size = 110
gvolume = GVolume("root")
gvolume.description = "World"
gvolume.make_box(world_size * 0.5, world_size * 0.5, world_size * 0.5)
gvolume.material = "G4_AIR"
gvolume.color = "ghostwhite"
gvolume.style = 0
gvolume.publish(configuration)

target_z = 20
target_radius = 5
gvolume = GVolume("Target")
gvolume.mother = "root"
gvolume.description = "Simple Carbon Target"
gvolume.make_tube(0, target_radius, target_z, 0, 360)
gvolume.material = "G4_C"
gvolume.color = "metallic, darkgreen"
gvolume.publish(configuration)

flux_z = 50
flux_dx = 1
flux_dim = world_size * 0.8
gvolume = GVolume("FluxPlane")
gvolume.mother = "root"
gvolume.description = "Flux Scoring Plane"
gvolume.make_box(flux_dim * 0.5, flux_dim * 0.5, flux_dx * 0.5)
gvolume.material = "G4_AIR"
gvolume.color = "FAFAD2"
gvolume.set_position(0, 0, flux_z)
gvolume.digitization = "flux"
gvolume.set_identifier("flux_plane", 1)
gvolume.publish(configuration)