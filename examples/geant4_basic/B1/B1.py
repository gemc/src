#!/usr/bin/env python3

# python:
import argparse
import logging

# api:
from utils_api import GConfiguration, defined_parser

# B1 specific
from materials import define_materials
from geometry import build_geometry

VARIATIONS = {
	"default",
}


def main():
	logging.basicConfig(level=logging.DEBUG)

	# Provides the -h, --help message
	args = defined_parser('The B1 system')

	for variation in VARIATIONS:
		# Define GConfiguration name, factory and description.
		configuration = GConfiguration('B1', 'TEXT', args)
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
