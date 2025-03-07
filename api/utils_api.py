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


from gemc_sqlite import create_sqlite_database
import sqlite3
import os, argparse, sys


def get_arguments():
	parser = argparse.ArgumentParser(description="GEMC Configuration Utility")
	parser.add_argument("-v", "--variation", default="default", help="Set variation")
	parser.add_argument("-r", "--run", default="1", help="Set run number")
	parser.add_argument("-f", "--factory", default="SQLITE", help="Default is SQLITE")
	parser.add_argument("-sql", "--sqlfilename", help="SQLite filename")
	return parser.parse_args()


# Configuration class definition
class GConfiguration():
	def __init__(self, system):

		# Get command-line arguments
		args = get_arguments()

		self.system = system
		self.runno = args.run
		self.factory = args.factory  # Use factory argument
		self.dbhost = 'gemc.sqlite'  # Default dbhost
		if args.sqlfilename:
			self.dbhost = args.sqlfilename
		self.sqlitedb: sqlite3.Connection = None
		self.verbosity = 0
		self.nvolumes = 0
		self.nmaterials = 0
		self.geoFileName = None
		self.matFileName = None
		self.mirFileName = None

		# Set variation
		self.setVariation(args.variation)
		self.init_sqlite_file(self.dbhost)

	def setVariation(self, newVariation):
		self.variation = newVariation
		print(f"Variation set to: {self.variation}")
		# filenames
		if self.factory == "TEXT":
			self.geoFileName = self.system + "__geometry_" + str(self.variation) + ".txt"
			self.matFileName = self.system + "__materials_" + str(self.variation) + ".txt"
			self.mirFileName = self.system + "__mirrors_" + str(self.variation) + ".txt"
		elif self.factory == "JSON":
			self.geoFileName = self.system + "__geometry_" + str(self.variation) + ".json"
			self.matFileName = self.system + "__materials_" + str(self.variation) + ".json"
			self.mirFileName = self.system + "__mirrors_" + str(self.variation) + ".json"

	def setRunNo(self, runno):
		self.runno = runno

	def setVerbosity(self, verbosity):
		self.verbosity = verbosity

	def init_mysql_host(self, dbhost):
		self.dbhost = dbhost

	def init_sqlite_file(self, sqlitedb_file):
		print()
		# Check if the database file already exists
		if not os.path.exists(sqlitedb_file):
			print(f"  ❖ Database file {sqlitedb_file} does not exist. Creating a new database...")
			try:
				self.sqlitedb = sqlite3.connect(sqlitedb_file)
				create_sqlite_database(self.sqlitedb)  # Initialize the database
				print(f"  ❖ Created new SQLite database: {sqlitedb_file}")
			except sqlite3.Error as e:
				sys.exit(f"Error creating SQLite database: {e}")
		else:
			print(f"  ❖ SQLite database file {sqlitedb_file} already exists. Connecting to it...")
			try:
				self.sqlitedb = sqlite3.connect(sqlitedb_file)
			except sqlite3.Error as e:
				sys.exit(f"Error connecting to SQLite database: {e}")


	def close_sqlite_file(self):
		self.sqlitedb.close()


	def printC(self):
		print(f"\n    ❖ GConfiguration for system <{GColors.BOLD}{self.system}{GColors.END}> : ")
		print(f"    ▪︎ Factory: {self.factory}")
		print(f"    ▪︎ Variation: {self.variation}")

		if self.factory == "MYSQL":
			if self.dbhost is None:
				sys.exit(' Error: MYSQL dbhost is not defined. Exiting.')
			else:
				print(f"    ▪︎ Host: {self.dbhost}")
		elif self.factory == "SQLITE":
			if self.sqlitedb is None:
				sys.exit(' Error: SQLITE db file is not defined. Exiting.')
			else:
				print(f"    ▪︎ SQLITE db : {self.sqlitedb}")
		print(f"    ▪︎ Variation: {self.variation}")

		if self.nvolumes > 0:
			print(f"    ▪︎ Number of volumes: {self.nvolumes}")
		if self.nmaterials > 0:
			print(f"    ▪︎ Number of materials: {self.nmaterials}")
		print()


		# overwrites any existing geometry file.
	def init_geom_file(self):
		if self.factory in ["TEXT", "JSON"]:
			try:
				with open(self.geoFileName, "w") as file:
					pass  # just to open and overwrite
			except OSError as e:
				sys.exit(f"Error opening file {self.geoFileName}: {e}")


	# overwrites any existing material file.
	def init_mats_file(self):
		if self.factory in ["TEXT", "JSON"]:
			try:
				with open(self.matFileName, "w") as file:
					pass  # just to open and overwrite
			except OSError as e:
				sys.exit(f"Error opening file {self.matFileName}: {e}")


	# overwrites any existing mirrors file.
	def init_mirs_file(self):
		if self.factory in ["TEXT", "JSON"]:
			try:
				with open(self.mirFileName, "w") as file:
					pass  # just to open and overwrite
			except OSError as e:
				sys.exit(f"Error opening file {self.mirFileName}: {e}")



# The following code allows this module to be executed as a main python script for the purpose of testing the functions
# To test, type:  'python utils.py' on the command line
if __name__ == "__main__":
	system1 = GConfiguration("ctof")
	system2 = GConfiguration("dc")
	system2.setVariation("rga_fall2019")

	system1.printC()
	system2.printC()
