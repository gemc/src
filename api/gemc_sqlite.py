#!/usr/bin/env python3

# Purposes:
# 1. function to create a sqlite database file with the geometry and materials tables
# 2. functions to fill the tables with the geometry and materials of a system

import argparse
import sys
import sqlite3
from utils_api import GColors

NGIVEN: str = 'NOTGIVEN'
NGIVENS: [str] = ['NOTGIVEN']


def main():
	# Provides the -h, --help message
	desc_str = "   gemc sqlite interface\n"
	sqlitedb: sqlite3.Connection = None

	variation_filter = ''
	system_filter = ''
	runno_filter = ''

	what = "*"

	parser = argparse.ArgumentParser(description=desc_str)

	# file writers
	parser.add_argument('-sql', action='store', type=str, help='set the sqlite filename',
	                    default=NGIVEN)
	parser.add_argument('-sv', action='store_true', help='show volumes from database')
	parser.add_argument('-sm', action='store_true', help='show materials from database')
	parser.add_argument('-ef', action='store', type=str,
	                    help='selects an experiment filter for the volumes')
	parser.add_argument('-vf', action='store', type=str,
	                    help='selects a variation filter for the volumes')
	parser.add_argument('-sf', action='store', type=str,
	                    help='selects a system filter for the volumes')
	parser.add_argument('-rf', action='store', type=str,
	                    help='selects a run number filter for the volumes')
	parser.add_argument('-what', action='store', type=str, help='show only the selected fields')

	args = parser.parse_args()

	if args.sql != NGIVEN:
		sqlitedb = sqlite3.connect(args.sql)

	if args.ef:
		experiment_filter = f" WHERE experiment = '{args.ef}'"

	if args.vf:
		if args.ef:
			variation_filter = f" and variation = '{args.vf}'"
		else:
			variation_filter = f" WHERE variation = '{args.vf}'"

	if args.sf:
		if args.vf or args.ef:
			system_filter = f" and system = '{args.sf}'"
		else:
			system_filter = f" WHERE system = '{args.sf}'"

	if args.rf:
		if args.vf or args.sf or args.ef:
			runno_filter = f" and run = {args.rf}"
		else:
			runno_filter = f' WHERE run = {args.rf}'

	all_filters = experiment_filter + variation_filter + system_filter + runno_filter

	if args.what:
		what = args.what

	if args.sv:
		show_volumes_from_database(sqlitedb, what, all_filters)

	if args.sm:
		show_materials_from_database(sqlitedb, what, all_filters)

	# if no argument is given print help
	if len(sys.argv) == 1:
		parser.print_help(sys.stderr)
		print()
		sys.exit(1)


def show_volumes_from_database(sqlitedb, what, all_filters):
	if sqlitedb is not None:
		sql = sqlitedb.cursor()
		query = "SELECT {} FROM geometry {};".format(what, all_filters)
		print(query)
		sql.execute(query)
		for row in sql.fetchall():
			print(row)


def show_materials_from_database(sqlitedb, what, all_filters):
	if sqlitedb is not None:
		sql = sqlitedb.cursor()
		query = "SELECT {} FROM materials {};".format(what, all_filters)
		print(query)
		sql.execute(query)
		for row in sql.fetchall():
			print(row)


# create the database file (overwrite if it exists)
# create the tables geometry, materials, mirrors, parameters
def create_sqlite_database(sqlitedb):
	sql = sqlitedb.cursor()

	# Create geometry table with one column
	sql.execute('''CREATE TABLE geometry
                 (id integer primary key)''')

	# Create materials table with one column
	sql.execute('''CREATE TABLE materials
                 (id integer primary key)''')

	# Save (commit) the changes
	sqlitedb.commit()


def add_geometry_fields_to_sqlite_if_needed(gvolume, configuration):
	# check if the geometry table has the geometry columns
	sql = configuration.sqlitedb.cursor()
	sql.execute("SELECT name FROM PRAGMA_TABLE_INFO('geometry');")
	fields = sql.fetchall()

	# if there is only one column, add the columns
	if len(fields) == 1:
		add_column(configuration.sqlitedb, "geometry", "experiment", "TEXT")
		add_column(configuration.sqlitedb, "geometry", "system", "TEXT")
		add_column(configuration.sqlitedb, "geometry", "variation", "TEXT")
		add_column(configuration.sqlitedb, "geometry", "run", "INTEGER")

		# add columns from gvolume class
		for field in gvolume.__dict__:
			if field != "gcolor":
				sql_type = sqltype_of_variable(gvolume.__dict__[field])
				add_column(configuration.sqlitedb, "geometry", field, sql_type)
	configuration.sqlitedb.commit()


def add_materials_fields_to_sqlite_if_needed(gmaterial, configuration):
	# check if the geometry table has the geometry columns
	sql = configuration.sqlitedb.cursor()
	sql.execute("SELECT name FROM PRAGMA_TABLE_INFO('materials');")
	fields = sql.fetchall()

	# if there is only one column, add the columns
	if len(fields) == 1:
		add_column(configuration.sqlitedb, "materials", "experiment", "TEXT")
		add_column(configuration.sqlitedb, "materials", "system", "TEXT")
		add_column(configuration.sqlitedb, "materials", "variation", "TEXT")
		add_column(configuration.sqlitedb, "materials", "run", "INTEGER")
		# add columns from gmaterial class
		for field in gmaterial.__dict__:
			if field != 'compType' and field != 'totComposition':
				sql_type = sqltype_of_variable(gmaterial.__dict__[field])
				add_column(configuration.sqlitedb, "materials", field, sql_type)
	configuration.sqlitedb.commit()


# Store already deleted (system, variation, runno) combinations so we delete only once
deleted_geometry = set()
deleted_materials = set()


def columns_and_values(gobject, configuration):
	cols = ["experiment", "system", "variation", "run"]
	vals = [configuration.experiment, configuration.system, configuration.variation,
	        configuration.runno]

	for field, value in gobject.__dict__.items():
		# keep same skips you already use elsewhere
		if field in ('compType', 'totComposition', 'gcolor'):
			continue
		cols.append(field)
		vals.append(value if not isinstance(value, list) else str(value))  # just in case

	return cols, tuple(vals)


def populate_sqlite_geometry(gvolume, configuration):
	global deleted_geometry
	add_geometry_fields_to_sqlite_if_needed(gvolume, configuration)

	sql = configuration.sqlitedb.cursor()
	key = (configuration.experiment, configuration.system, configuration.variation,
	       configuration.runno)

	sql.execute("SELECT count(*) FROM sqlite_master WHERE type='table' AND name='geometry'")
	if sql.fetchone()[0] != 1:
		sys.exit(f"{GColors.RED}Error: 'geometry' table does not exist.{GColors.END}")

	if key not in deleted_geometry:
		sql.execute(
			"""DELETE FROM geometry WHERE experiment=? AND system=? AND variation=? AND run=?""",
			key)
		configuration.sqlitedb.commit()
		deleted_geometry.add(key)

	# make sure rotations is a string before inserting
	gvolume.rotations = gvolume.get_rotation_string()

	sql.execute(
		"""SELECT COUNT(*) FROM geometry 
		   WHERE name=? AND experiment=? AND system=? AND variation=? AND run=?""",
		(gvolume.name, configuration.experiment, configuration.system, configuration.variation,
		 configuration.runno)
	)
	if sql.fetchone()[0] != 0:
		sys.exit(
			f"{GColors.RED}Error: volume >{gvolume.name}< already exists for variation '{configuration.variation}'{GColors.END}")

	cols, vals = columns_and_values(gvolume, configuration)
	placeholders = ",".join(["?"] * len(cols))
	sql.execute(f"INSERT INTO geometry ({','.join(cols)}) VALUES ({placeholders})", vals)
	configuration.sqlitedb.commit()


def populate_sqlite_materials(gmaterial, configuration):
	global deleted_materials
	add_materials_fields_to_sqlite_if_needed(gmaterial, configuration)

	sql = configuration.sqlitedb.cursor()
	key = (configuration.experiment, configuration.system, configuration.variation,
	       configuration.runno)

	if key not in deleted_materials:
		sql.execute(
			"""DELETE FROM materials WHERE experiment=? AND system=? AND variation=? AND run=?""",
			key)
		configuration.sqlitedb.commit()
		deleted_materials.add(key)

	sql.execute(
		"""SELECT COUNT(*) FROM materials 
		   WHERE name=? AND experiment=? AND system=? AND variation=? AND run=?""",
		(gmaterial.name, configuration.experiment, configuration.system, configuration.variation,
		 configuration.runno)
	)
	if sql.fetchone()[0] != 0:
		sys.exit(
			f"{GColors.RED}Error: material >{gmaterial.name}< already exists for variation '{configuration.variation}'{GColors.END}")

	cols, vals = columns_and_values(gmaterial, configuration)
	placeholders = ",".join(["?"] * len(cols))
	sql.execute(f"INSERT INTO materials ({','.join(cols)}) VALUES ({placeholders})", vals)
	configuration.sqlitedb.commit()


def form_string_with_column_definitions(gobject) -> str:
	strn = "( experiment, system, variation, run, "
	for field in gobject.__dict__:
		if field != 'compType' and field != 'totComposition' and field != 'gcolor':
			# print(field)
			strn += f"{field}, "
	strn = strn[:-2] + ")"
	return strn


def form_string_with_column_values(gobject, configuration) -> str:
	strn = f"( '{configuration.experiment}', '{configuration.system}', '{configuration.variation}', {configuration.runno}, "
	for field, value in gobject.__dict__.items():
		if field != 'compType' and field != 'totComposition' and field != 'gcolor':
			strn += f"'{value}', " if isinstance(value, str) else f"{value}, "
	strn = strn[:-2] + ")"  # Remove last comma and space, then close parenthesis
	return strn


def sqltype_of_variable(variable) -> str:
	if type(variable) is int:
		return 'INT'
	elif type(variable) is str:
		return 'TEXT'


def add_column(db, tablename, column_name, var_type):
	sql = db.cursor()
	strn = "ALTER TABLE {0} ADD COLUMN {1} {2}".format(tablename, column_name, var_type)
	# print(strn)
	sql.execute(strn)
	db.commit()


if __name__ == "__main__":
	main()
