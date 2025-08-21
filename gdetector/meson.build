sub_dir_name = meson.current_source_dir().split('/').get(-1)

verbosities = ['-verbosity.plugins=2',
               '-debug.plugins=true',
               '-verbosity.gsystem=2',
               '-debug.gsystem=true',
               '-verbosity.gworld=2',
               '-debug.gworld=true',
               '-verbosity.gvolume=2',
               '-debug.gvolume=true',
               '-verbosity.g4system=2',
               '-debug.g4system=true',
               '-verbosity.g4system=2',
               '-debug.g4system=true',
               '-verbosity.gdetector=2',
               '-debug.gdetector=true'
]

example_source = files('examples/gdetector_example.cc')
examples_dir = get_option('prefix') + '/test/' + sub_dir_name
dbhost = ['-sql=' + examples_dir + '/gemc.db']
gsystem = ['-gsystem="[{name: b1}]"']

LD += {
    'name' : sub_dir_name,
    'sources' : files(
        'gdetectorConstruction.cc',
        'gdetector_options.cc'
    ),
    'headers' : files(
        'gdetectorConstruction.h',
        'gdetector_options.h'
    ),
    'dependencies' : [yaml_cpp_dep, clhep_deps, geant4_deps],
    'geo_build' : {
        'test_detector_geo' : [files('examples/example.py')]
    },
    'examples' : {
        'test_gdetector' : [example_source, gsystem + dbhost],
        'test_gdetector_verbose' : [example_source, gsystem + dbhost + verbosities]
    }
}
