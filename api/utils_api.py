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

from typing import Optional

has_pyvista: bool = False
pv: Optional[object] = None  # will hold the module if available

try:
	import pyvista as _pv

	has_pyvista = True
except ModuleNotFoundError:
	pass  # leave defaults


def get_arguments():
	parser = argparse.ArgumentParser(description="GEMC Configuration Utility")
	parser.add_argument("-f", "--factory", default="sqlite", help="ascii, sqlite")
	parser.add_argument("-v", "--variation", default="default", help="Set variation")
	parser.add_argument("-r", "--run", default=1, help="Set run number")
	parser.add_argument("-sql", "--dbhost", default='gemc.db', help="SQLite filename or MYSQL host")
	# pyvista
	parser.add_argument("-pv", "--pyvista", action="store_true", help="Show geometry using pyvista")
	parser.add_argument("-pvw", "--width", type=int, default=2400, help="Set plotter width")
	parser.add_argument("-pvh", "--height", type=int, default=1600, help="Set plotter height")
	parser.add_argument("-pvx", "--x", type=int, default=0, help="Set plotter x position")
	parser.add_argument("-pvy", "--y", type=int, default=0, help="Set plotter y position")
	parser.add_argument("-axes", "--add_axes_at_zero", action="store_true", help="Add 10cm axes at (0, 0, 0)")
	return parser.parse_args()


# Configuration class definition
class GConfiguration:
	def __init__(self, experiment, system, factory=None, variation=None, runno=1, verbosity=0):
		self.args = get_arguments()  # expose args to scripts that use this class
		self.experiment = experiment
		self.system = system
		self.runno = self.args.run if self.args.run else runno
		self.factory = self.args.factory if self.args.factory else factory  # Prioritize command-line argument
		self.variation = self.args.variation if self.args.variation else variation  # Prioritize command-line argument
		self.dbhost = self.args.dbhost
		self.sqlitedb: sqlite3.Connection = None
		self.verbosity = verbosity
		self.nvolumes = 0
		self.nmaterials = 0
		self.geoFileName = None
		self.matFileName = None
		self.mirFileName = None
		if self.args.pyvista and not has_pyvista:
			print(f"{GColors.RED}Error: pyvista requested but not installed. Exiting.{GColors.END}")
			sys.exit(1)
		self.use_pyvista = self.args.pyvista and has_pyvista
		self.pv = _pv if self.use_pyvista else None
		self._plotter: Optional[object] = None

		self.init_variation(self.variation)
		self.initialize_storage()

	@property
	def plotter(self):
		"""Lazily create a single Plotter and keep it for the session."""
		if self.pv is None:
			return None
		if self._plotter is None:
			self._plotter = self.pv.Plotter()
			# do one-time scene setup here
			self._plotter.add_axes()
			self._plotter.set_background("black")
			self._plotter.camera_position = "iso"
		return self._plotter

	def add_mesh(self, *args, **kwargs):
		"""Convenience pass-through; safe if pv disabled."""
		p = self.plotter
		if p is None:
			return None
		actor = p.add_mesh(*args, **kwargs)
		return actor

	def add_origin_axes(self, L=10.0, width=3):
		if self.pv is None:
			return
		p = self.plotter

		xline = self.pv.Line((-L, 0, 0), (L, 0, 0))
		yline = self.pv.Line((0, -L, 0), (0, L, 0))
		zline = self.pv.Line((0, 0, -L), (0, 0, L))
		p.add_mesh(xline, color="red", line_width=width)
		p.add_mesh(yline, color="green", line_width=width)
		p.add_mesh(zline, color="blue", line_width=width)

	def clear(self):
		"""Optional: clear scene without destroying the plotter."""
		if self._plotter is not None:
			self._plotter.clear()

	def close(self):
		"""Optional: fully close and release the plotter."""
		if self._plotter is not None:
			self._plotter.close()
			self._plotter = None

	def init_variation(self, newVariation):
		if self.variation != newVariation:
			self.variation = newVariation
			print(f"  ❖ Variation switched to: {self.variation}")
		# filenames
		if self.factory == "ascii":
			self.geoFileName = self.system + "__geometry_" + str(self.variation) + ".txt"
			self.matFileName = self.system + "__materials_" + str(self.variation) + ".txt"
			self.mirFileName = self.system + "__mirrors_" + str(self.variation) + ".txt"

	# overwrites any existing geometry file.
	def initialize_storage(self):
		print()
		if self.factory in ['sqlite']:
			sqlite_file = self.dbhost
			# Check if the database file already exists
			if not os.path.exists(sqlite_file):
				print(f"  ❖ Database file {sqlite_file} does not exist")
				try:
					self.sqlitedb = sqlite3.connect(sqlite_file)
					create_sqlite_database(self.sqlitedb)  # Initialize the database
					print(f"  ❖ Created new SQLite database: {sqlite_file}")
				except sqlite3.Error as e:
					sys.exit(f"Error creating SQLite database: {e}")
			else:
				try:
					self.sqlitedb = sqlite3.connect(sqlite_file)
					cursor = self.sqlitedb.cursor()
					cursor.execute("SELECT name FROM sqlite_master WHERE type='table' AND name='geometry'")
					if cursor.fetchone() is None:
						print(f"  ❖ Warning: Database exists but missing required tables")
						self.sqlitedb.close()
						os.remove(sqlite_file)
						print(f"  ❖ Removed incomplete database: {sqlite_file}")
						self.sqlitedb = sqlite3.connect(sqlite_file)
						create_sqlite_database(self.sqlitedb)
						print(f"  ❖ Created new SQLite database with required tables")

				except sqlite3.Error as e:
					sys.exit(f"Error connecting to SQLite database: {e}")

		elif self.factory in ["ascii"]:
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

	def show(self):
		print()
		print(
			f"  ❖  GConfiguration for experiment <{GColors.BOLD}{self.experiment}{GColors.END}>,  system <{GColors.BOLD}{self.system}{GColors.END}> : ")

		print(f"	▪︎ Factory: {self.factory:<15}")

		if self.factory == "mysql":
			if not self.dbhost:
				sys.exit(f"{GColors.RED}Error: MYSQL dbhost is not defined. Exiting.{GColors.END}")
			print(f"	▪︎ MySQL Host: {self.dbhost}")

		elif self.factory == "sqlite":
			if not self.sqlitedb:
				sys.exit(f"{GColors.RED}Error: sqlite db file is not defined. Exiting.{GColors.END}")
			print(f"	▪︎ SQLite File: {self.args.dbhost if self.args.dbhost else 'default.db'}")

		elif self.factory in ["ascii"]:
			print(f"    	↦ Geometry File: {self.geoFileName}")
			print(f"    	↦ Materials File: {self.matFileName}")
			print(f"    	↦ Mirrors File: {self.mirFileName}")

		print(f"	▪︎ (Variation, Run): ({self.variation}, {self.runno})")

		if self.nvolumes > 0:
			print(f"	▪︎ Number of volumes: {self.nvolumes}")
		if self.nmaterials > 0:
			print(f"	▪︎ Number of materials: {self.nmaterials}")
		print()
		if self.use_pyvista:
			p = self._plotter
			if p is not None:
				w, h, x, y = self.args.width, self.args.height, self.args.x, self.args.y
				p.ren_win.SetPosition(x, y)
				p.ren_win.SetSize(w, h)
				try:
					import numpy as np

					# compute a good camera distance from the scene bounds
					xmin, xmax, ymin, ymax, zmin, zmax = p.bounds
					center = np.array([(xmin + xmax) / 2, (ymin + ymax) / 2, (zmin + zmax) / 2])
					scene_len = np.linalg.norm([xmax - xmin, ymax - ymin, zmax - zmin])
					direction = np.array([1, 1, 1]) / np.sqrt(3)  # iso direction
					distance = 2.5 * scene_len  # increase factor to zoom further out

					pos = center + direction * distance
					p.camera_position = [tuple(pos), tuple(center), (0, 0, 1)]
				except ImportError:
					pass

				if self.args.add_axes_at_zero:
					self.add_origin_axes()
				p.view_zy()  # or p.view_xz(), p.view_yz()
				p.enable_anti_aliasing()  # FXAA
				p.show()


# The following code allows this module to be executed as a main python script for the purpose of testing the functions
# To test, type:  'python utils.py' on the command line
if __name__ == "__main__":
	system1 = GConfiguration("examples", "ctof")
	system1.show()

	system2 = GConfiguration("examples", "dc")
	system2.show()
