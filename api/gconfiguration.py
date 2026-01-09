#!/usr/bin/env python3.13

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

# python
import sqlite3
import os, argparse, sys

from dataclasses import dataclass
from typing import Optional, Tuple

# gemc api
from gsqlite import create_sqlite_database
from gutils import GColors

has_pyvista: bool = False
pv: Optional[object] = None
BackgroundPlotterCls = None

try:
	import pyvista as _pv
	import numpy as np

	has_pyvista = True
	pv = _pv
	try:
		from pyvistaqt import BackgroundPlotter as _BackgroundPlotter

		BackgroundPlotterCls = _BackgroundPlotter
	except ModuleNotFoundError:
		BackgroundPlotterCls = None
except ModuleNotFoundError:
	pass


def get_arguments(argv=None):
	parser = argparse.ArgumentParser(description="GEMC Configuration Utility")
	parser.add_argument("-f", "--factory", default="sqlite", help="ascii, sqlite")
	parser.add_argument("-v", "--variation", default="default", help="Set variation")
	parser.add_argument("-r", "--run", default=1, help="Set run number")
	parser.add_argument("-sql", "--dbhost", default='gemc.db', help="SQLite filename or MYSQL host")
	# pyvista
	parser.add_argument("-pv", "--pyvista", action="store_true", help="Show geometry using pyvista")
	parser.add_argument(
		"-pvb", "--pvb", "--pyvista-background",
		dest="pyvista_background",
		action="store_true",
		help="Use PyVista BackgroundPlotter (non-blocking GUI, implies --pyvista)",
	)
	parser.add_argument("-pvw", "--width", type=int, default=2400, help="Set plotter width")
	parser.add_argument("-pvh", "--height", type=int, default=1600, help="Set plotter height")
	parser.add_argument("-pvx", "--x", type=int, default=0, help="Set plotter x position")
	parser.add_argument("-pvy", "--y", type=int, default=0, help="Set plotter y position")
	parser.add_argument("-axes", "--add_axes_at_zero", action="store_true",
	                    help="Add 10cm axes at (0, 0, 0)")

	if argv is None:
		# Safe default: ignore unknown IPython/Jupyter args
		args, _ = parser.parse_known_args()
	else:
		args = parser.parse_args(argv)

	return args


# Configuration class definition
class GConfiguration:
	def __init__(
			self,
			experiment,
			system,
			factory=None,
			variation=None,
			runno=1,
			verbosity=0,
			args=None,
			enable_pyvista: Optional[bool] = None,
			use_background_plotter: bool = None,
	):
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

		# pyvista
		# CLI background flag
		background_flag = bool(getattr(self.args, "pyvista_background", False))

		# Decide if PyVista is wanted:
		#   - programmatic enable_pyvista overrides CLI (True/False)
		#   - otherwise: --pyvista OR --pvb imply pyvista
		if enable_pyvista is None:
			wants_pyvista = self.args.pyvista or background_flag
		else:
			wants_pyvista = enable_pyvista

		if wants_pyvista and not has_pyvista:
			print(f"{GColors.RED}Error: pyvista requested but not installed. Exiting.{GColors.END}")
			sys.exit(1)

		self.use_pyvista = wants_pyvista and has_pyvista
		self.pv = pv if self.use_pyvista else None

		# Decide whether to use BackgroundPlotter:
		#   - programmatic use_background_plotter overrides CLI (True/False)
		#   - otherwise: --pvb controls it
		if use_background_plotter is None:
			self.use_background_plotter = background_flag
		else:
			self.use_background_plotter = use_background_plotter

		self._plotter: Optional[object] = None
		self._camera_initialized = False

		self.init_variation(self.variation)
		self.initialize_storage()

	@property
	def plotter(self):
		if not self.use_pyvista or self.pv is None:
			return None

		if self._plotter is None:
			if self.use_background_plotter and BackgroundPlotterCls is not None:
				# Non-blocking background window
				self._plotter = BackgroundPlotterCls(show=True)
			else:
				# Normal blocking Plotter
				self._plotter = self.pv.Plotter()

			# One-time scene setup
			self._plotter.add_axes()
			self._plotter.set_background("#303048", top="#000020")
			self._plotter.camera_position = "iso"

		return self._plotter

	def add_mesh(self, *args, **kwargs):
		p = self.plotter
		if p is None:
			return None
		actor = p.add_mesh(*args, **kwargs)

		# For BackgroundPlotter, optionally configure camera once after first mesh
		if self.use_background_plotter and not self._camera_initialized:
			self._configure_camera_from_bounds()

		return actor

	def add_origin_axes(self, L=50.0, width=3):
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
					cursor.execute(
						"SELECT name FROM sqlite_master WHERE type='table' AND name='geometry'")
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

	def show(self, block: bool = True):
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
				sys.exit(
					f"{GColors.RED}Error: sqlite db file is not defined. Exiting.{GColors.END}")
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
			p = self.plotter
			if p is not None:
				# window position/size if available
				w, h, x, y = self.args.width, self.args.height, self.args.x, self.args.y
				try:
					p.ren_win.SetPosition(x, y)
					p.ren_win.SetSize(w, h)
				except AttributeError:
					pass  # BackgroundPlotter may not expose ren_win directly

				if self.use_background_plotter:
					# BackgroundPlotter path
					if block and hasattr(p, "app"):
						# Block here in scripts when requested
						p.app.exec_()
				else:
					# Normal Plotter path
					if block:
						p.show()

	def _configure_camera_from_bounds(self):
		if not self.use_pyvista:
			return

		p = self.plotter
		if p is None:
			return

		# If nothing is added yet, bounds can be degenerate
		try:
			xmin, xmax, ymin, ymax, zmin, zmax = p.bounds
		except Exception:
			return

		# Guard against empty scene
		if any(not np.isfinite(v) for v in (xmin, xmax, ymin, ymax, zmin, zmax)):
			return
		if xmax == xmin and ymax == ymin and zmax == zmin:
			return

		center = np.array([(xmin + xmax) / 2,
		                   (ymin + ymax) / 2,
		                   (zmin + zmax) / 2])
		scene_len = np.linalg.norm([xmax - xmin,
		                            ymax - ymin,
		                            zmax - zmin])

		# iso direction
		direction = np.array([1.0, 1.0, 1.0]) / np.sqrt(3.0)
		distance = 2.5 * scene_len if scene_len > 0 else 1.0

		pos = center + direction * distance
		p.camera_position = [tuple(pos), tuple(center), (0, 0, 1)]

		if self.args.add_axes_at_zero:
			# if you have this helper on the plotter; otherwise use your own
			try:
				p.add_axes_at_origin(xlabel="X", ylabel="Y", zlabel="Z")
			except AttributeError:
				pass

		# Optional: keep your original “nice” orientation
		try:
			p.view_zy()  # or view_isometric, etc.
		except AttributeError:
			pass

		try:
			p.enable_anti_aliasing()
		except AttributeError:
			pass

		try:
			p.enable_parallel_projection()
		except AttributeError:
			pass

		# Sometimes helps with clipping issues
		if hasattr(p, "reset_camera_clipped_range"):
			p.reset_camera_clipped_range()

		self._camera_initialized = True


# autogeometry utility to executes show() at exit
import atexit

"""
Returns a GConfiguration immediately and arranges .show() at process exit.
"""


def autogeometry(
		experiment: str,
		application: str,
		auto_show: bool = True,
		enable_pyvista: Optional[bool] = None,
		use_background_plotter: Optional[bool] = None,
):
	cfg = GConfiguration(
		experiment,
		application,
		enable_pyvista=enable_pyvista,
		use_background_plotter=use_background_plotter,
	)

	if auto_show:
		# For scripts, we *want* to block on exit (both -pv and -pvb)
		atexit.register(lambda: cfg.show(block=True))

	return cfg
