#!/usr/bin/python -B   # -B prevents writing .pyc compiled versions of each module

# TODO: use argparse for options
# TODO: add sconstruct
# TODO: 

import sys, getopt, string

validRoutineNames = ["constants",       "loadTT",                 "processID",                "hitDigitization",  "all"]
validRoutineDescr = ["loads constants", "loat translation table", "manipulate/create new ID", "digitize a hit",   "write all routines"]

def printHelp():
	print ''
	print 'Usage:'
	print ''
	print 'createSystemRoutine.py -s <systemName> -r [routineNames]'
	print '[routineNames] is a vector of names separated by space'
	print ''
	print 'Example: createSystemRoutine.py -s driftChamber -r constants touchable'
	print ''
	print 'Available routines:'
	for n in range(len(validRoutineNames)):
		print '-', validRoutineNames[n], ':',validRoutineDescr[n]
	print ''


def parseSystem(argv):
	sysName = "none"
	try:
		opts, args = getopt.getopt(argv,"h:s:r:h")
	except getopt.GetoptError:
		printHelp()
		sys.exit(2)
	for opt, arg in opts:
		if opt == '-h':
			printHelp()
			sys.exit()
		elif opt == "-s":
			sysName = arg
       		return sysName
	return sysName

def parseRNames(argv):
	opts, args = getopt.getopt(argv,"h:s:r")
	rNames = [""]
	for opt, arg in opts:
		if opt == "-r":
			rNames = args
			return rNames
	return rNames

def validateOptions(system, routines):
	if system == 'none':
		print ''
		print 'Error: system not given, use -s to define it'
		printHelp()
		sys.exit()
	for roname in routines:
		if not roname in validRoutineNames :
			print ''
			print 'Error: ', roname, ' is not a vaild routine name'
			printHelp()
			sys.exit()


def writeHeader(sName, routines):
	headerFileName = sName + '.h'
	headerFile = open(headerFileName, 'w')
	headerFile.write('#ifndef ' + sName.upper() + 'PLUGIN\n')
	headerFile.write('#define ' + sName.upper() + 'PLUGIN 1\n')
	headerFile.write('\n')
	headerFile.write('// glibrary\n')
	headerFile.write('#include "gdynamicdigitization.h"\n')
	headerFile.write('#include "gutsConventions.h"\n')
	headerFile.write('\n')
	headerFile.write('class ' + sName + 'Plugin : public GDynamicDigitization {\n')
	headerFile.write('\n')
	headerFile.write('public:\n')
	headerFile.write('\n')

	headerFile.write('\t// mandatory readout specs definitions\n')
	headerFile.write('\tbool defineReadoutSpecs();\n')

	if 'constants' in routines:
		headerFile.write('\n')
		headerFile.write('\t// loads digitization constants\n')
		headerFile.write('\tbool loadConstants(int runno, string variation);\n')

	if 'loadTT' in routines:
		headerFile.write('\n')
		headerFile.write('\t// loads the translation table\n')
		headerFile.write('\tbool loadTT(int runno, string variation);\n')

	if 'hitDigitization' in routines:
		headerFile.write('\n')
		headerFile.write('\t// digitized the hit\n')
		headerFile.write('\tGDigitizedData* digitizeHit(GHit *ghit, int hitn);\n')

	headerFile.write('\n')
	headerFile.write('private:\n')
	headerFile.write('\n')
	headerFile.write('\t// constants definitions\n')
	headerFile.write('\n')
	headerFile.write('};\n')
	headerFile.write('\n')
	headerFile.write('#endif\n')
	headerFile.write('\n')
	headerFile.close()


# PRAGMA TODO: add comments/documentation
# PRAGMA TODO: add commented example
def writeLoadConstants(sName):
	constantsFile = open('loadConstants.cc', 'w')
	constantsFile.write('#include "' + sName + '.h"\n')
	constantsFile.write('\n')
	constantsFile.write('bool ' + sName + 'Plugin::loadConstants(int runno, string variation)\n')
	constantsFile.write('{\n')
	constantsFile.write('\n')
	constantsFile.write('\n')
	constantsFile.write('\n')
	constantsFile.write('\treturn true;\n')
	constantsFile.write('}\n')

# PRAGMA TODO: add comments/documentation
# PRAGMA TODO: add commented example
def writeLoadTT(sName):
	constantsFile = open('loadTT.cc', 'w')
	constantsFile.write('#include "' + sName + '.h"\n')
	constantsFile.write('\n')
	constantsFile.write('bool ' + sName + 'Plugin::loadTT(int runno, string variation)\n')
	constantsFile.write('{\n')
	constantsFile.write('\n')
	constantsFile.write('\ttranslationTable = new GTranslationTable();\n')
	constantsFile.write('\n')
	constantsFile.write('\n')
	constantsFile.write('\treturn true;\n')
	constantsFile.write('}\n')


# PRAGMA TODO: add comments/documentation
# PRAGMA TODO: add commented example
def writeHitDigitization(sName):
	constantsFile = open('hitDigitization.cc', 'w')
	constantsFile.write('#include "' + sName + '.h"\n')
	constantsFile.write('\n')
	constantsFile.write('GDigitizedData* ' + sName + 'Plugin::digitizeHit(GHit *ghit, int hitn)\n')
	constantsFile.write('{\n')
	constantsFile.write('\tGDigitizedData* gdata = new GDigitizedData();\n')
	constantsFile.write('\n')
	constantsFile.write('\n')
	constantsFile.write('\n')
	constantsFile.write('\n')
	constantsFile.write('\n')
	constantsFile.write('\n')
	constantsFile.write('\n')
	constantsFile.write('\tgdata->includeVariable("hitn",  hitn);\n')
	constantsFile.write('\treturn gdata;\n')
	constantsFile.write('}\n')


# PRAGMA TODO: load the file existing in the directory
def writeSConstruct(sName, routines):
	sconsFile = open('SConstruct', 'w')
	sconsFile.write('\n')
	sconsFile.write('\n')
	sconsFile.write('\n')
	sconsFile.write('\n')
	sconsFile.write('\n')
	sconsFile.write('\n')
	sconsFile.write('\n')
	sconsFile.write('\n')
	sconsFile.write('\n')

# PRAGMA TODO: add comments/documentation
# PRAGMA TODO: add commented example
def writeReadoutSpects(sName):
	readoutSpecs = open('readoutSpecs.cc', 'w')
	readoutSpecs.write('#include "' + sName + '.h"\n')
	readoutSpecs.write('\n')
	readoutSpecs.write('bool ' + sName + 'Plugin::defineReadoutSpecs()\n')
	readoutSpecs.write('{\n')
	readoutSpecs.write('\tfloat     timeWindow = 10;                  // electronic readout time-window of the detector\n')
	readoutSpecs.write('\tfloat     gridStartTime = 0;                // defines the windows grid\n')
	readoutSpecs.write('\tHitBitSet hitBitSet = HitBitSet("000000");  // defines what information to be stored in the hit\n')
	readoutSpecs.write('\tbool      verbosity = true;\n')
	readoutSpecs.write('\n')
	readoutSpecs.write('\treadoutSpecs = new GReadoutSpecs(timeWindow, gridStartTime, hitBitSet, verbosity);\n')
	readoutSpecs.write('\n')
	readoutSpecs.write('\treturn true;\n')
	readoutSpecs.write('}\n')
	readoutSpecs.write('\n')
	readoutSpecs.write('\n')
	readoutSpecs.write('// DO NOT EDIT BELOW THIS LINE: defines how to create the <' + sName + 'Plugin>\n')
	readoutSpecs.write('extern "C" GDynamicDigitization* GDynamicFactory(void) {\n')
	readoutSpecs.write('\treturn static_cast<GDynamicDigitization*>(new ' + sName + 'Plugin);\n')
	readoutSpecs.write('}\n')
	readoutSpecs.write('\n')


# parsing and writing sources
systemName = parseSystem(sys.argv[1:])
routines   = parseRNames(sys.argv[1:])

validateOptions(systemName, routines)

# or use createNewHeader function
writeHeader(systemName, routines)
writeReadoutSpects(systemName)

if 'constants' in routines or 'all' in routines:
	writeLoadConstants(systemName)

if 'loadTT' in routines or 'all' in routines:
	writeLoadTT(systemName)

if 'hitDigitization' in routines or 'all' in routines:
	writeHitDigitization(systemName)




