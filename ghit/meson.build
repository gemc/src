sub_dir_name = meson.current_source_dir().split('/').get(-1)
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
    'dependencies' : [yaml_cpp_dep, clhep_deps, geant4_deps],
    'examples' : {
        'test_ghit' : [files('examples/ghit_example.cc'), ''],
        'test_ghit_verbose' : [files('examples/ghit_example.cc'), '-verbosity.gtouchable=2', '-debug.gtouchable=true']
    }
}
