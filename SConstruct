from init_env import init_environment

# load needed environment for this library
# xercesc needed just on posix  by geant4
env = init_environment('qt5 glibrary geant4 clhep  xercesc')
env.Append(CXXFLAGS=['-std=c++20'])

# local directories include
env.Append(CPPPATH = [Dir('.'), Dir('utilities'), Dir('gsession'), Dir('gsd'), Dir('gdetector'), Dir('userActions'), Dir('gui')])

# gemc subsystems are built in 'lib'
SConscript('utilities/SConscript',   exports='env')
SConscript('gsession/SConscript',    exports='env')
SConscript('gsd/SConscript',         exports='env')
SConscript('gdetector/SConscript',   exports='env')
SConscript('userActions/SConscript', exports='env')

SConscript('gui/SConscript',         exports='env')

# load needed environment for this library
env.Append(LIBPATH = ['lib'])

gsources = [
	'gemc.cc',
	'gemcOptions.cc',
	'qtresources.qrc'
	]

# build gemc
gemc = env.Program(source = gsources, target = 'gemc')
