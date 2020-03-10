from init_env import init_environment

env = init_environment("qt5 glibrary geant4 clhep")
env.Append(CXXFLAGS=['-std=c++17'])
env.Append(LIBPATH = ['lib'])

env.Append(CPPPATH = [Dir('utilities')])


# utilities
SConscript('utilities/SConscript', exports='env')

gsources = Split("""
	gemc.cc
	""")

gemc = env.Program(source = gsources, target = "gemc")

