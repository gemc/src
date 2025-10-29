#!/usr/bin/env python3

# -*- coding: utf-8 -*-
# =======================================
#	gemc utils definition
#
#	This file defines a GConfiguration class that holds a GEMC system configuration.
#
#
#	Class members (all members are text strings):
#	system   	- The name of the system. Think project name here.
#	variation 	- The name of the project variation.  For example, one could have variations of the project where
#					- a volume has a different size or material.  The variation defaults to 'default'
#	factory		- The configuration factory defines how the generated files that gemc uses are stored.
#					- Possible choices: TEXT, MYSQL, JSON
#	dbhost		- The hostname of the mysql database server where gemc detectors, materials, etc. may be stored
#					- for the MYSQL factory.
#	verbosity	- The log verbosity level for the sci-g API. The default is 0 (print only summary information)

# color utility for logging
class GColors:
	PURPLE = '\033[95m'
	CYAN = '\033[96m'
	DARKCYAN = '\033[36m'
	BLUE = '\033[94m'
	GREEN = '\033[92m'
	YELLOW = '\033[93m'
	RED = '\033[91m'
	BOLD = '\033[1m'
	UNDERLINE = '\033[4m'
	END = '\033[0m'

