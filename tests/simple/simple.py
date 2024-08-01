#!/usr/bin/env python3

# python:
import argparse
import logging
# api:
from utils_api import GConfiguration

# simple:
from materials import define_materials
from geometry import build_geometry

VARIATIONS = {
    "default",
}

def main():
	logging.basicConfig(level=logging.DEBUG)

	# Provides the -h, --help message
	desc_str = "   Will create the simple system\n"
	parser = argparse.ArgumentParser(description=desc_str)
	args = parser.parse_args()

	for variation in VARIATIONS:

		# Define GConfiguration name, factory and description.
		configuration = GConfiguration('simple', 'TEXT', 'The simple system')
		configuration.setVariation(variation)

		# define materials
		configuration.init_mats_file()
		define_materials(configuration)

		# build geometry
		configuration.init_geom_file()
		build_geometry(configuration)

		# print out the GConfiguration
		configuration.printC()


if __name__ == "__main__":
	main()


