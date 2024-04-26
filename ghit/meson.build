sub_dir_name = meson.current_source_dir().split('/').get(-1)
LD += {
    'name' : sub_dir_name,
    'sources' : files([
                          'ghit.cc',
                          'calculations.cc',
                          'addHitInfos.cc'
                      ]),
    'headers' : files([
                          'ghit.h',
                          'ghitConventions.h'
                      ]),
    'dependencies' : [geant4_deps, clhep_deps],
    'examples' : {
        'basic_hit' : [files(['examples/simple.cc']), ['']]
    }
}
