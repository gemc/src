from init_env import init_environment

# load needed environment for this library
env = init_environment('qt5 glibrary geant4 clhep')
env.Append(CXXFLAGS=['-std=c++17'])

# local directories include
env.Append(CPPPATH = [Dir('utilities'), Dir('glog')])

# gemc subsystems are built in 'lib'
SConscript('utilities/SConscript', exports='env')
SConscript('glog/SConscript',      exports='env')


# load needed environment for this library
env.Append(LIBPATH = ['lib'])

gsources = Split("""
	gemc.cc
	qtresources.qrc
	""")

# build gemc
gemc = env.Program(source = gsources, target = "gemc")

