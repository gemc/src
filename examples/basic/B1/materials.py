from materials_api import GMaterial

def define_materials(configuration):

# example of material: epoxy glue, defined with number of atoms
	gmaterial = GMaterial("epoxy")
	gmaterial.description = "epoxy glue 1.16 g/cm3"
	gmaterial.density = 1.16
	gmaterial.addNAtoms("H",  32)
	gmaterial.addNAtoms("N",   2)
	gmaterial.addNAtoms("O",   4)
	gmaterial.addNAtoms("C",  15)
	gmaterial.publish(configuration)

# example of material: carbon fiber, defined using the fractional mass
	gmaterial = GMaterial("carbonFiber")
	gmaterial.description = "carbon fiber - 1.75g/cm3"
	gmaterial.density = 1.75
	gmaterial.addMaterialWithFractionalMass("G4_C",  0.745)
	gmaterial.addMaterialWithFractionalMass("epoxy", 0.255)
	gmaterial.publish(configuration)



