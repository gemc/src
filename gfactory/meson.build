sub_dir_name = meson.current_source_dir().split('/').get(-1)

example_sourcess = files('examples/static_and_dynamic_example.cc', 'examples/ShapeFactory.cc')

verbosities = ['-verbosity.plugins=2',
               '-debug.plugins=true'
]

LD += {
    'name' : sub_dir_name,
    'sources' : files('gfactory_options.cc'),
    'headers' : files(
        'gdl.h',
        'gfactory.h',
        'gfactory_options.h'
    ),
    'dependencies' : [yaml_cpp_dep, clhep_deps, geant4_deps],
    'plugins' : {
        'test_dyn_factory1' : [files('examples/FordFactory.cc'), true],
        'test_dyn_factory2' : [files('examples/TeslaFactory.cc'), true],
    },
    'examples' : {
        'test_gfactory_static_and_dynamic_load' : [example_sourcess, ''],
        'test_gfactory_static_and_dynamic_load_verbose' : [example_sourcess, verbosities]
    }
}
