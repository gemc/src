sub_dir_name = meson.current_source_dir().split('/').get(-1)
internal_deps = []

example_source = files('examples/ghit_example.cc')
verbosities = ['-verbosity.gtouchable=2',
               '-debug.gtouchable=true'
]


LD += {
    'name' : sub_dir_name,
    'sources' : files(
        'ghit.cc',
        'calculations.cc',
        'addHitInfos.cc'
    ),
    'headers' : files(
        'ghit.h',
        'ghitConventions.h'
    ),

    'dependencies' : [yaml_cpp_dep, clhep_deps, geant4_core_deps],
    'internal_dependencies' : internal_deps,

    'examples' : {
        'test_ghit' : [example_source, ''],
        'test_ghit_verbose' : [example_source, verbosities]
    }
}
