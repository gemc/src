#!/usr/bin/env python3

# gemc api:
from utils_api import GConfiguration

# B2
from geometry import build_geometry
#from materials import define_materials

def main():
	configuration = GConfiguration("examples", "b2")
	build_geometry(configuration)
#	define_materials(configuration)
	configuration.printC()

if __name__ == "__main__":
	main()
