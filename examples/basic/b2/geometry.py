from geometry_api import GVolume
import math


# These are example of methods to build a mother and daughter volume.

def build_geometry(configuration):
	fNbOfChambers = 5
	chamberSpacing = 800  # from chamber center to center

	chamberWidth = 200.0  # width of the chambers
	targetLength = 50.0  # full length of Target

	trackerLength = (fNbOfChambers + 1) * chamberSpacing
	worldLength = 1.2 * (2 * targetLength + trackerLength)

	targetRadius = 0.5 * targetLength  # Radius of Target
	targetLength = 0.5 * targetLength  # Half length of the Target
	trackerSize = 0.5 * trackerLength  # Half length of the Tracker

	gvolume = GVolume('root')
	gvolume.description = 'World'
	gvolume.make_box(worldLength * 0.5, worldLength * 0.5, worldLength * 0.5)
	gvolume.material = 'G4_AIR'
	gvolume.color = 'ghostwhite'
	gvolume.style = 0
	gvolume.publish(configuration)

	tz = -(targetLength + trackerSize)
	gvolume = GVolume('target')
	gvolume.description = 'Lead Target'
	gvolume.make_tube(0, targetRadius, targetLength, 0, 360)
	gvolume.material = 'G4_Pb'
	gvolume.set_position(0, 0, tz)
	gvolume.color = 'steelblue'
	gvolume.publish(configuration)

	gvolume = GVolume('tracker')
	gvolume.description = 'Tracker'
	gvolume.make_tube(0, trackerSize, trackerSize, 0, 360)
	gvolume.material = 'G4_AIR'
	gvolume.visible = 0
	gvolume.publish(configuration)


	firstPosition = -trackerSize + chamberSpacing
	firstLength = trackerLength / 10
	lastLength = trackerLength
	halfWidth = 0.5 * chamberWidth
	rmaxFirst = 0.5 * firstLength

	rmaxIncr = 0.5 * (lastLength - firstLength) / (fNbOfChambers - 1)

	# loop from 0 to fNbOfChambers
	for copyNo in range(fNbOfChambers):
		Zposition = firstPosition + copyNo * chamberSpacing
		rmax = rmaxFirst + copyNo * rmaxIncr
		gvolume = GVolume(f'Chamber_PV_{copyNo}')
		gvolume.description = 'Tracker'
		gvolume.make_tube(0, rmax, halfWidth, 0, 360)
		gvolume.material = 'G4_Xe'
		gvolume.color = 'papayawhip'
		gvolume.set_position(0, 0, Zposition)
		gvolume.publish(configuration)