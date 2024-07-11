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
    'dependencies' : [clhep_deps, geant4_deps],
    'examples' : {
        'test_basic_hit' : [files('examples/simple.cc'), '']
    }
}
