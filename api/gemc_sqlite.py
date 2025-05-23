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
	parser.add_argument('-sql',  action='store', type=str, help='set the sqlite filename', default=NGIVEN)
	parser.add_argument('-sv',   action='store_true', help='show volumes from database')
	parser.add_argument('-sm',   action='store_true', help='show materials from database')
	parser.add_argument('-ef',   action='store', type=str, help='selects an experiment filter for the volumes')
	parser.add_argument('-vf',   action='store', type=str, help='selects a variation filter for the volumes')
	parser.add_argument('-sf',   action='store', type=str, help='selects a system filter for the volumes')
	parser.add_argument('-rf',   action='store', type=str, help='selects a run number filter for the volumes')
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

def populate_sqlite_geometry(gvolume, configuration):
	global deleted_geometry

	add_geometry_fields_to_sqlite_if_needed(gvolume, configuration)

	sql = configuration.sqlitedb.cursor()
	key = (configuration.experiment, configuration.system, configuration.variation, configuration.runno)

	# Check if the geometry table exists
	sql.execute("SELECT count(*) FROM sqlite_master WHERE type='table' AND name='geometry'")
	if sql.fetchone()[0] == 1:  # Table exists
		if key not in deleted_geometry:
			delete_query = """
                DELETE FROM geometry WHERE experiment = ? AND system = ? AND variation = ? AND run = ?
            """
			sql.execute(delete_query, key)
			configuration.sqlitedb.commit()
			deleted_geometry.add(key)
	else:
		print("Warning: 'geometry' table does not exist. Skipping deletion.")

	# Updated query: Check existence based on name **AND** variation/run/experiment/system
	sql.execute(
		"""SELECT COUNT(*) FROM geometry 
		   WHERE name = ? AND experiment = ? AND system = ? AND variation = ? AND run = ?""",
		(gvolume.name, configuration.experiment, configuration.system, configuration.variation, configuration.runno)
	)

	if sql.fetchone()[0] == 0:
		columns = form_string_with_column_definitions(gvolume)
		values = form_string_with_column_values(gvolume, configuration)
		sql.execute(f"INSERT INTO geometry {columns} VALUES {values}")
	else:
		sys.exit(f"{GColors.RED}Error: volume >{gvolume.name}< already exists in the database for variation '{configuration.variation}'{GColors.END}")

	configuration.sqlitedb.commit()





def populate_sqlite_materials(gmaterial, configuration):
	global deleted_materials  # Ensure we modify the global set

	add_materials_fields_to_sqlite_if_needed(gmaterial, configuration)

	sql = configuration.sqlitedb.cursor()

	key = (configuration.experiment, configuration.system, configuration.variation, configuration.runno)

	# Ensure deletion happens only once per variation and run
	if key not in deleted_materials:
		delete_query = """
            DELETE FROM materials WHERE experiment = ? AND system = ? AND variation = ? AND run = ?
        """
		sql.execute(delete_query, key)
		configuration.sqlitedb.commit()  # Ensure deletion applies
		deleted_materials.add(key)  # Mark as deleted

	# Check if material already exists for this experiment, system, variation, and run
	sql.execute(
		"""SELECT COUNT(*) FROM materials 
		   WHERE name = ? AND experiment = ? AND system = ? AND variation = ? AND run = ?""",
		(gmaterial.name, configuration.experiment, configuration.system, configuration.variation, configuration.runno)
	)

	if sql.fetchone()[0] == 0:
		columns = form_string_with_column_definitions(gmaterial)
		values = form_string_with_column_values(gmaterial, configuration)

		insert_query = f"INSERT INTO materials {columns} VALUES {values}"
		sql.execute(insert_query)
		configuration.sqlitedb.commit()
	else:
		sys.exit(f"{GColors.RED}Error: material >{gmaterial.name}< already exists in the database for variation '{configuration.variation}'{GColors.END}")



def form_string_with_column_definitions(gobject) -> str:
	strn = "( experiment, system, variation, run, "
	for field in gobject.__dict__:
		if field != 'compType' and field != 'totComposition':
			# print(field)
			strn += f"{field}, "
	strn = strn[:-2] + ")"
	return strn


def form_string_with_column_values(gobject, configuration) -> str:
	strn = f"( '{configuration.experiment}', '{configuration.system}', '{configuration.variation}', {configuration.runno}, "
	for field, value in gobject.__dict__.items():
		if field != 'compType' and field != 'totComposition':
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
	#print(strn)
	sql.execute(strn)
	db.commit()


if __name__ == "__main__":
	main()
