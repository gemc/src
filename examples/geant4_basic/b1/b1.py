#!/usr/bin/env python3

# gemc api:
from utils_api import GConfiguration

# B1 specific
from materials import define_B1_materials
from geometry import build_B1_geometry


def main():
	configuration = GConfiguration("examples", "b1")
	build_B1_geometry(configuration)
	define_B1_materials(configuration)
	configuration.printC()

if __name__ == "__main__":
	main()
