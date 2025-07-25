sub_dir_name = meson.current_source_dir().split('/').get(-1)

examples_dir = get_option('prefix') + '/test/' + sub_dir_name
dbhost = '-sql=' + examples_dir + '/gemc.db'
gsystem = '-gsystem="[{name: b1}]"'

LD += {
    'name' : sub_dir_name,
    'sources' : files(
        'g4volume.cc',
        'g4world.cc',
        'g4system_options.cc',
        'g4objectsFactories/geant4Dependencies.cc',
        'g4objectsFactories/g4objectsFactory.cc',
        'g4objectsFactories/g4native/checkAndReturnParameters.cc',
        'g4objectsFactories/g4native/buildSolid.cc',
        'g4objectsFactories/cad/buildSolid.cc',
    ),
    'headers' : files(
        'g4volume.h',
        'g4world.h',
        'g4system_options.h',
        'g4systemConventions.h',
        'g4objectsFactories/g4objectsFactory.h',
        'g4objectsFactories/g4native/g4NativeObjectsFactory.h',
        'g4objectsFactories/cad/CADMesh.hh',
        'g4objectsFactories/cad/cadSystemFactory.h',
    ),
    'dependencies' : [yaml_cpp_dep, clhep_deps, geant4_deps, assimp_dep],
    'additional_includes' : ['g4system/g4objectsFactories', 'g4system/g4objectsFactories/g4native', 'g4system/g4objectsFactories/cad'],
    'geo_build' : {
        'test_g4system_geo' : [files('examples/example.py')]
    },
    'examples' : {
        'test_g4system' : [files('examples/g4system_example.cc'), [gsystem, dbhost]],
        'test_g4system_verbose' : [files('examples/g4system_example.cc'), [gsystem, dbhost, '-verbosity.g4system=2', '-debug.g4system=true']],
        'test_g4system_overlaps_switch' : [files('examples/g4system_example.cc'), [gsystem, dbhost, '-check_overlaps=1']]
    }
}
