sub_dir_name = meson.current_source_dir().split('/').get(-1)
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
        'test_gfactory_static_and_dynamic_load' : [files('examples/static_and_dynamic_example.cc', 'examples/ShapeFactory.cc'), ''],
        'test_gfactory_static_and_dynamic_load_with_debug' : [files('examples/static_and_dynamic_example.cc', 'examples/ShapeFactory.cc'), '-debug.plugins=true'],
        'test_gfactory_static_and_dynamic_load_with_verbosity' : [files('examples/static_and_dynamic_example.cc', 'examples/ShapeFactory.cc'), '-verbosity.plugins=2']
    }
}
