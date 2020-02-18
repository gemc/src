from init_env import init_environment

env = init_environment("geant4")
env.Append(LIBPATH = ['lib'])


gsources = Split("""
	gemc.cc
	""")

gemc = env.Program(source = gsources, target = "gemc")

