from geometry_api import GVolume
import math


# These are example of methods to build a mother and daughter volume.

def build_geometry(configuration):
	cryst_dX = 60
	cryst_dY = 60
	cryst_dZ = 30
	nb_cryst = 32
	nb_rings = 9

	dPhi = 360 / nb_cryst
	half_dPhi = 0.5 * dPhi

	cosdPhi = math.cos(half_dPhi*math.pi/180.)
	tandPhi = math.tan(half_dPhi*math.pi/180.)

	ring_R1 = 0.5 * cryst_dY / tandPhi
	ring_R2 = (ring_R1 + cryst_dZ) / cosdPhi
	detector_dZ = nb_rings * cryst_dX

	world_sizeXY = 2.4 * ring_R2
	world_sizeZ = 1.2 * detector_dZ

	gvolume = GVolume('root')
	gvolume.description = 'World'
	gvolume.make_box(world_sizeXY * 0.5, world_sizeXY * 0.5, world_sizeZ * 0.5)
	gvolume.material = 'G4_AIR'
	gvolume.color = 'ghostwhite'
	gvolume.style = 0
	gvolume.publish(configuration)

	gvolume = GVolume('Detector')
	gvolume.description = 'Detector'
	gvolume.make_tube(ring_R1, ring_R2, 0.5* detector_dZ, 0, 360)
	gvolume.material = 'G4_AIR'
	gvolume.visible = 0
	gvolume.publish(configuration)

	OG = -0.5 * (detector_dZ + cryst_dX)

	for icrys in range(nb_rings):
		# phi_rot = icrys * dPhi
		#
		# c = (ring_R1 + 0.5 * cryst_dZ)
		# posx = c *  math.cos(phi_rot*math.pi/180.)
		# posy = c *  math.sin(phi_rot*math.pi/180.)
		OG += cryst_dX

		gvolume = GVolume(f'Ring{icrys}')
		gvolume.description = f'Ring{icrys}'
		gvolume.make_tube(ring_R1, ring_R2, 0.5* cryst_dX, 0, 360)
		gvolume.set_position(0, 0, OG)
		gvolume.material = 'G4_AIR'
		gvolume.visible = 0
		gvolume.publish(configuration)