# -*- coding: utf-8 -*-
#=======================================
#	gemc materials definition
#
#	This file defines a GMaterial class that holds the parameters needed to define a Geant4 material in gemc.
#	Any material in the project is an instance of this class.
#  The "publish" function writes out the volume parameters according to the factory.
#
#  A GMaterial is be instantiated with these mandatory arguments:
#
#	Class members (all members are text strings):
#
#	name			  - The name of the material
#	density		  - The material density, in g/cm3
#	components	  - A string that lists the components and their relative amounts in the material, e.g. "H 2 O 1"
#
#	The following GMaterial parameters are optional: FIX ME: NOT SUPPORTED YET
#
#	description	  - A description of the material, for documentation purposes only
#
#	***** Optional Optical properties:
#
#	photonEnergy      - A list of photon energies with units, at which any other optical parameters will be evaluated
#				 	      - if any optical parameter (indexOfRefraction, reflectivity, etc.) is defined, photonEnergy MUST also be defined
#					      - example:  "1.0*eV 2.0*eV 3.0*eV 4.0*eV 5.0*eV 6.0*eV"
#
#	indexOfRefraction	- A list of the refractive index evaluated at the energies named in photonEnergy
#					      - must have same number of elements in list as in photonEnergy
#					      - example:  "1.40 1.39 1.38 1.37 1.36"
#
#	absorptionLength	- A list of the material absorption length with units, evaluated at the energies in photonEnergy
#					      - must have same number of elements in list as in photonEnergy
#					      - example:  "72.8*m 53.2*cm 39.1*cm"
#
#	reflectivity		- A list of reflectivity values evaluated at the energies in photonEnergy
#					      - must have same number of elements in list as in photonEnergy
#					      - example:  "0.2 0.5"
#
#	efficiency			- A list of absorption efficiency evaluated at the energies in photonEnergy
#					      - efficiency is only used for a dielectric-metal optical boundary where there is no refraction
#					      - At this boundary the photon is either reflected or absorbed by the metal with this efficiency
#					      - This parameter can be used to define a quantum efficiency for a PMT, for example
#
#
#	***** Optional Scintillation properties:
#
#	Scintillators are assumed to have a fast and slow response component, defined by relative spectra
#  All quantities given as a single number should be positive, so their unassigned value is set to -1
#
#	fastcomponent		   - A list of the fast component relative spectra values evaluated at the energies in photonEnergy
#	slowcomponent		   - A list of the fast component relative spectra values evaluated at the energies in photonEnergy
#	scintillationyield	- Characteristic light yield in photons/MeV e-, given as a single number
#	resolutionscale		- Resolution scale broadens the statistical distribution of generated photons
#					         - due to impurities typical of doped crystals like NaI(Tl) and CsI(Tl).  Can be narrower
#					         - when the Fano factor plays a role.  Actual number of emitted photons in a step fluctuates
#					         - around the mean number with width (ResolutionScale*sqrt(MeanNumberOfPhotons)
#					         - Resolution scale is given as a single number
#	fasttimeconstant	   - FIX ME believe this is related to the scintillator pulse rise time. Given as single number, in nanosecond
#	slowtimeconstant	   - FIX ME believe this is related to scintillator slow decay time. Given as single number, in nanosecond
#	yieldratio			   - relative strength of the fast component as a fraction of total scintillation yield, given as a single number
#	yieldratio			   - relative strength of the fast component as a fraction of total scintillation yield, given as a single number
#	birksConstant		   - FIX ME
#
#	***** Other opticsl properties:
#
#	rayleigh	- A list of the Rayleigh scattering attenuation coefficient evaluated at the energies in photonEnergy
#
#
#
#	******	Note that photon energies can be obtained from the wavelength:
#
#			lambda * nu = c	where lambda is wavelength, c is the speed of light, and nu is frequency
#			E = h * nu		   where h is Plank's constant
#			A handy relation for estimating is that h*c ~ 197 eV*nm

import sys, math

# for mandatory fields. Used in function check_validity
WILLBESETSTRING     = 'notSetYet'
WILLBESETNUMBER     = -987654

# for optionals fields
NOTASSIGNEDSTRING = 'na'
NOTASSIGNEDNUMBER = -1

ISCHEMICAL   = "ISCHEMICAL"
ISFRACTIONAL = "ISFRACTIONAL"

from scig_sql import populate_sqlite_materials

# Material class definition
class GMaterial():
	def __init__(self, name):

		# mandatory fields. Checked at publish time
		self.name        = name
		self.density     = WILLBESETNUMBER
		self.composition = WILLBESETSTRING
		# compType is not part of the material definition - it's used for validation
		# if addNAtoms is used: ISCHEMICAL
		# if addMaterialWithFractionalMass: ISFRACTIONAL
		# 1. if ISCHEMICAL totComposition must be > 1
		# 2. if ISFRACTIONAL totComposition must be = 1
		self.compType       = WILLBESETSTRING
		self.totComposition = 0

		# optional fields (have default values)
		self.description        = NOTASSIGNEDSTRING

		# optical parameters
		self.photonEnergy       = NOTASSIGNEDSTRING
		self.indexOfRefraction  = NOTASSIGNEDSTRING
		self.absorptionLength   = NOTASSIGNEDSTRING
		self.reflectivity       = NOTASSIGNEDSTRING
		self.efficiency         = NOTASSIGNEDSTRING

		# scintillation parameters
		self.fastcomponent      = NOTASSIGNEDSTRING
		self.slowcomponent      = NOTASSIGNEDSTRING
		self.scintillationyield = NOTASSIGNEDNUMBER
		self.resolutionscale    = NOTASSIGNEDNUMBER
		self.fasttimeconstant   = NOTASSIGNEDNUMBER
		self.slowtimeconstant   = NOTASSIGNEDNUMBER
		self.yieldratio         = NOTASSIGNEDNUMBER
		self.birksConstant      = NOTASSIGNEDNUMBER

		# other optical processes
		self.rayleigh           = NOTASSIGNEDSTRING

	def check_validity(self):
		# need to add checking if it's operation instead
		if self.density == WILLBESETNUMBER:
			sys.exit(' Error: density not defined for GMaterial '    + str(self.name) )
		if self.composition == WILLBESETSTRING:
			sys.exit(' Error: components not defined for GMaterial ' + str(self.name) )
		if self.compType == WILLBESETSTRING:
			sys.exit(' Error: composition type not defined for GMaterial ' + str(self.name) )
		if self.compType == ISCHEMICAL:
			if self.totComposition <= 1:
				sys.exit(' Error: chemical formula has total composition less or equal 1  for material: ' + str(self.name) )
		if self.compType == ISFRACTIONAL:
			if not math.isclose(self.totComposition, 1, rel_tol=1e-6):
				sys.exit(' Error: fractional masses do not add to 1 for material: ' + str(self.name) )

	def publish(self, configuration):
		self.check_validity()
		# TEXT factory
		if configuration.factory == 'TEXT':
			fileName = configuration.matFileName
			configuration.nmaterials += 1
			with open(fileName, 'a+') as dn:
				lstr = ''
				lstr += '%s | ' % self.name
				lstr += '%s | ' % self.density
				lstr += '%s | ' % self.composition
				lstr += '%s | ' % self.description

				# optical parameters
				lstr += '%s | ' % self.photonEnergy
				lstr += '%s | ' % self.indexOfRefraction
				lstr += '%s | ' % self.absorptionLength
				lstr += '%s | ' % self.reflectivity
				lstr += '%s | ' % self.efficiency

				# scintillation parameters
				lstr += '%s | ' % self.fastcomponent
				lstr += '%s | ' % self.slowcomponent
				lstr += '%s | ' % self.scintillationyield
				lstr += '%s | ' % self.resolutionscale
				lstr += '%s | ' % self.fasttimeconstant
				lstr += '%s | ' % self.slowtimeconstant
				lstr += '%s | ' % self.yieldratio
				lstr += '%s | ' % self.birksConstant

				# other optical processes
				lstr += '%s |\n' % self.rayleigh

				dn.write(lstr)
		# SQLITE factory
		elif configuration.factory == 'SQLITE':
			configuration.nmaterials += 1
			populate_sqlite_materials(self, configuration)

	def addNAtoms(self, element, natoms):
		if self.composition == WILLBESETSTRING:
			self.composition = element + ' '
			self.compType    = ISCHEMICAL
		else:
			self.composition += element + ' '
		self.composition += str(natoms) + ' '
		self.totComposition += natoms

	def addMaterialWithFractionalMass(self, material, fractionalMass):
		if self.composition == WILLBESETSTRING:
			self.composition = material + ' '
			self.compType    = ISFRACTIONAL
		else:
			self.composition += material + ' '
		self.composition += str(fractionalMass) + ' '
		self.totComposition += fractionalMass
