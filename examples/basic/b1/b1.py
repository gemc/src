#!/usr/bin/env python3

# gemc api:
from utils_api import GConfiguration

# B1
from geometry import build_geometry


def main():
	configuration = GConfiguration("examples", "b1")
	build_geometry(configuration)
	configuration.show()

if __name__ == "__main__":
	main()
