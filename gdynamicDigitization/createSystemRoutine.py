#!/usr/bin/env python3
"""
Script to generate plugin C++ source and header files based on given system name and routines.
"""

import argparse

# Define valid routine names and their descriptions
VALID_ROUTINE_NAMES = ["constants", "loadTT", "processID", "hitDigitization", "all"]
VALID_ROUTINE_DESCR = {
	"constants": "loads constants",
	"loadTT": "loads translation table",
	"processID": "manipulate/create new ID",
	"hitDigitization": "digitizes a hit",
	"all": "write all routines"
}

def get_arguments():
	parser = argparse.ArgumentParser(
		description="Generate system routine files.",
		epilog=f"Example: %(prog)s -s driftChamber -r constants hitDigitization\n"
			   f"Available routines: {', '.join(VALID_ROUTINE_NAMES)}"
	)
	parser.add_argument(
		'-s', '--system',
		required=True,
		help="Name of the system (e.g., driftChamber)"
	)
	parser.add_argument(
		'-r', '--routines',
		nargs='+',
		required=True,
		choices=VALID_ROUTINE_NAMES,
		help="List of routine names to generate. Choices: " + ", ".join(VALID_ROUTINE_NAMES)
	)
	return parser.parse_args()

def write_header(system, routines):
	filename = f"{system}.h"
	with open(filename, "w") as header:
		header.write(f"#ifndef {system.upper()}PLUGIN\n")
		header.write(f"#define {system.upper()}PLUGIN 1\n\n")
		header.write("// glibrary\n")
		header.write('#include "gdynamicdigitization.h"\n')
		header.write('#include "gutsConventions.h"\n\n')
		header.write(f"class {system}Plugin : public GDynamicDigitization {{\n\n")
		header.write("public:\n\n")
		header.write("\t// mandatory readout specs definitions\n")
		header.write("\tbool defineReadoutSpecs();\n")
		if "constants" in routines or "all" in routines:
			header.write("\n\t// loads digitization constants\n")
			header.write("\tbool loadConstants(int runno, string variation);\n")
		if "loadTT" in routines or "all" in routines:
			header.write("\n\t// loads the translation table\n")
			header.write("\tbool loadTT(int runno, string variation);\n")
		if "hitDigitization" in routines or "all" in routines:
			header.write("\n\t// digitizes the hit\n")
			header.write("\tGDigitizedData* digitizeHit(GHit *ghit, int hitn);\n")
		header.write("\nprivate:\n\n")
		header.write("\t// constants definitions\n\n")
		header.write("};\n\n")
		header.write("#endif\n")

def write_load_constants(system):
	filename = "loadConstants.cc"
	with open(filename, "w") as f:
		f.write(f'#include "{system}.h"\n\n')
		f.write(f"bool {system}Plugin::loadConstants(int runno, string variation)\n")
		f.write("{\n")
		f.write("\n\t// TODO: add constants loading implementation\n\n")
		f.write("\treturn true;\n")
		f.write("}\n")

def write_load_tt(system):
	filename = "loadTT.cc"
	with open(filename, "w") as f:
		f.write(f'#include "{system}.h"\n\n')
		f.write(f"bool {system}Plugin::loadTT(int runno, string variation)\n")
		f.write("{\n")
		f.write("\n\t// TODO: add translation table loading implementation\n")
		f.write("\ttranslationTable = new GTranslationTable();\n\n")
		f.write("\treturn true;\n")
		f.write("}\n")

def write_hit_digitization(system):
	filename = "hitDigitization.cc"
	with open(filename, "w") as f:
		f.write(f'#include "{system}.h"\n\n')
		f.write(f"GDigitizedData* {system}Plugin::digitizeHit(GHit *ghit, int hitn)\n")
		f.write("{\n")
		f.write("\tGDigitizedData* gdata = new GDigitizedData();\n")
		f.write("\n\t// TODO: add hit digitization implementation\n")
		f.write('\tgdata->includeVariable("hitn", hitn);\n')
		f.write("\treturn gdata;\n")
		f.write("}\n")

def write_readout_specs(system):
	filename = "readoutSpecs.cc"
	with open(filename, "w") as f:
		f.write(f'#include "{system}.h"\n\n')
		f.write(f"bool {system}Plugin::defineReadoutSpecs()\n")
		f.write("{\n")
		f.write("\tfloat timeWindow = 10;                  // electronic readout time-window of the detector\n")
		f.write("\tfloat gridStartTime = 0;                // defines the window's grid start time\n")
		f.write('\tHitBitSet hitBitSet = HitBitSet("000000");  // defines what information to be stored in the hit\n')
		f.write("\tbool verbosity = true;\n\n")
		f.write("\treadoutSpecs = new GReadoutSpecs(timeWindow, gridStartTime, hitBitSet, verbosity);\n\n")
		f.write("\treturn true;\n")
		f.write("}\n\n")
		f.write(f"// DO NOT EDIT BELOW THIS LINE: defines how to create the <{system}Plugin>\n")
		f.write('extern "C" GDynamicDigitization* GDynamicFactory(void) {\n')
		f.write(f"\treturn static_cast<GDynamicDigitization*>(new {system}Plugin);\n")
		f.write("}\n")

def write_sconstruct(system, routines):
	filename = "SConstruct"
	with open(filename, "w") as f:
		f.write("# SConstruct file placeholder\n")
		f.write("# TODO: implement SConstruct content\n")

def main():
	args = get_arguments()
	system = args.system
	routines = args.routines

	write_header(system, routines)
	write_readout_specs(system)

	if "constants" in routines or "all" in routines:
		write_load_constants(system)
	if "loadTT" in routines or "all" in routines:
		write_load_tt(system)
	if "hitDigitization" in routines or "all" in routines:
		write_hit_digitization(system)

	# If you decide to support SConstruct generation unconditionally or conditionally:
	write_sconstruct(system, routines)

if __name__ == "__main__":
	main()
