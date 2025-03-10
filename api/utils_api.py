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


import sqlite3
import os, argparse, sys
from gemc_sqlite import create_sqlite_database


def get_arguments():
	parser = argparse.ArgumentParser(description="GEMC Configuration Utility")
	parser.add_argument("-v", "--variation", default="default", help="Set variation")
	parser.add_argument("-r", "--run", default=1, help="Set run number")
	parser.add_argument("-f", "--factory", default="SQLITE", help="SQLITE, ASCII or JSON")
	parser.add_argument("-sql", "--sqlfilename", default='gemc.db', help="SQLite filename")
	return parser.parse_args()


# Configuration class definition

class GConfiguration:
	def __init__(self, system, factory=None, variation=None, runno=1, verbosity=0):
		self.args = get_arguments() # expose args to scripts that use this class
		self.system = system
		self.runno = self.args.run if self.args.run else runno
		self.factory = self.args.factory if self.args.factory else factory  # Prioritize command-line argument
		self.variation = self.args.variation if self.args.variation else variation  # Prioritize command-line argument
		self.dbhost = self.args.sqlfilename
		self.sqlitedb: sqlite3.Connection = None
		self.verbosity = verbosity
		self.nvolumes = 0
		self.nmaterials = 0
		self.geoFileName = None
		self.matFileName = None
		self.mirFileName = None

		# Set variation
		self.init_variation(self.variation)
		self.initialize_storage()

	def init_variation(self, newVariation):
		if self.variation != newVariation:
			self.variation = newVariation
			print(f"  ❖ Variation switched to: {self.variation}")
		# filenames
		if self.factory == "ASCII":
			self.geoFileName = self.system + "__geometry_" + str(self.variation) + ".txt"
			self.matFileName = self.system + "__materials_" + str(self.variation) + ".txt"
			self.mirFileName = self.system + "__mirrors_" + str(self.variation) + ".txt"
		elif self.factory == "JSON":
			self.geoFileName = self.system + "__geometry_" + str(self.variation) + ".json"
			self.matFileName = self.system + "__materials_" + str(self.variation) + ".json"
			self.mirFileName = self.system + "__mirrors_" + str(self.variation) + ".json"


		# overwrites any existing geometry file.
	def initialize_storage(self):
		if self.factory in ['SQLITE']:
			sqlite_file=self.dbhost
			# Check if the database file already exists
			if not os.path.exists(sqlite_file):
				print(f"  ❖ Database file {sqlite_file} does not exist. Creating a new database...")
				try:
					self.sqlitedb = sqlite3.connect(sqlite_file)
					create_sqlite_database(self.sqlitedb)  # Initialize the database
					print(f"  ❖ Created new SQLite database: {sqlite_file}")
				except sqlite3.Error as e:
					sys.exit(f"Error creating SQLite database: {e}")
			else:
				try:
					self.sqlitedb = sqlite3.connect(sqlite_file)
				except sqlite3.Error as e:
					sys.exit(f"Error connecting to SQLite database: {e}")

		elif self.factory in ["ASCII", "JSON"]:
			try:
				with open(self.geoFileName, "w") as file:
					pass  # just to open and overwrite
			except OSError as e:
				sys.exit(f"Error opening file {self.geoFileName}: {e}")
			try:

				with open(self.matFileName, "w") as file:
					pass
			except OSError as e:
				sys.exit(f"Error opening file {self.matFileName}: {e}")

			try:
				with open(self.mirFileName, "w") as file:
					pass
			except OSError as e:
				sys.exit(f"Error opening file {self.mirFileName}: {e}")


	def printC(self):
		print()
		print(f"    ❖ GConfiguration for system <{GColors.BOLD}{self.system}{GColors.END}> : ")
		print(f"    ▪︎ Factory: {self.factory:<15}")

		if self.factory == "MYSQL":
			if not self.dbhost:
				sys.exit(f"{GColors.RED}Error: MYSQL dbhost is not defined. Exiting.{GColors.END}")
			print(f"    ▪︎ MySQL Host: {self.dbhost}")

		elif self.factory == "SQLITE":
			if not self.sqlitedb:
				sys.exit(f"{GColors.RED}Error: SQLITE db file is not defined. Exiting.{GColors.END}")
			print(f"    	↦ SQLite File: {self.args.sqlfilename if self.args.sqlfilename else 'default.db'}")

		elif self.factory in ["ASCII", "JSON"]:
			print(f"    	↦ Geometry File: {self.geoFileName}")
			print(f"    	↦ Materials File: {self.matFileName}")
			print(f"    	↦ Mirrors File: {self.mirFileName}")

		print(f"    ▪︎ (Variation, Run): ({self.variation}, {self.runno})")

		if self.nvolumes > 0:
			print(f"    ▪︎ Number of volumes: {self.nvolumes}")
		if self.nmaterials > 0:
			print(f"    ▪︎ Number of materials: {self.nmaterials}")
		print()


# The following code allows this module to be executed as a main python script for the purpose of testing the functions
# To test, type:  'python utils.py' on the command line
if __name__ == "__main__":
	system1 = GConfiguration("ctof")
	system1.printC()

	system2 = GConfiguration("dc")
	system2.printC()
