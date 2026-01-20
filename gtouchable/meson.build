sub_dir_name = meson.current_source_dir().split('/').get(-1)
internal_deps = []

example_source = files('examples/gtouchable_example.cc')

verbosities = ['-verbosity.gtouchable=2',
               '-debug.gtouchable=true'
]

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

    'dependencies' : [yaml_cpp_dep, clhep_deps, geant4_core_deps],
    'internal_dependencies' : internal_deps,

    'examples' : {
        'test_gtouchable_example' : [example_source, ''],
        'test_gtouchable_example_verbose' : [example_source, verbosities]
    }
}
