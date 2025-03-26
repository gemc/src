sub_dir_name = meson.current_source_dir().split('/').get(-1)
LD += {
    'name' : sub_dir_name,
    'sources' : files(
        'gelectronic.cc',
        'gtouchable.cc',
        'gtouchable_options.cc'

    ),
    'headers' : files(
        'gelectronic.h',
        'gtouchable.h',
        'gtouchableConventions.h',
        'gtouchable_options.h'

    ),
    'dependencies' : [yaml_cpp_dep, clhep_deps, geant4_deps],
    'examples' : {
        'test_gtouchable_example' : [files('examples/gtouchable_example.cc'), ''],
        'test_gtouchable_example_debug' : [files('examples/gtouchable_example.cc'), '-debug.gtouchable=1'],
        'test_gtouchable_example_verbosity' : [files('examples/gtouchable_example.cc'), '-verbosity.gtouchable=1']
    }
}
