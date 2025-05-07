from geometry_api import GVolume
import math


# These are example of methods to build a mother and daughter volume.

def build_geometry(configuration):

	gvolume = GVolume('Envelope')
	gvolume.description = 'B1 Envelope'
	gvolume.make_box(100.0, 100.0, 150.0)
	gvolume.material = 'G4_WATER'
	gvolume.color = '0000FF4'
	gvolume.publish(configuration)

	shape1_rmina = 0.0
	shape1_rmaxa = 20.0
	shape1_rminb = 0.0
	shape1_rmaxb = 40.0
	shape1_hz = 30.0
	shape1_phimin = 0.0
	shape1_phimax = 360.0

	gvolume = GVolume('Shape1')
	gvolume.mother = 'Envelope'
	gvolume.description = 'B1 Shape1'
	gvolume.make_cons(shape1_rmina, shape1_rmaxa, shape1_rminb, shape1_rmaxb, shape1_hz, shape1_phimin, shape1_phimax)
	gvolume.material = 'G4_A-150_TISSUE'
	gvolume.color = 'ffdddd'
	gvolume.set_position(0, 20, -70)
	gvolume.publish(configuration)

	shape2_dxa = 120/2
	shape2_dxb = 120/2
	shape2_dya = 100/2
	shape2_dyb = 160/2
	shape2_dz = 60/2

	gvolume = GVolume('Shape2')
	gvolume.mother = 'Envelope'
	gvolume.description = 'B1 Shape2'
	gvolume.make_trapezoid(shape2_dxa, shape2_dxb, shape2_dya, shape2_dyb, shape2_dz)
	gvolume.material = 'G4_BONE_COMPACT_ICRU'
	gvolume.color = 'ddffff'
	gvolume.set_position(0, -10, 70)
	gvolume.publish(configuration)
