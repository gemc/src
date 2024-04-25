sub_dir_name = meson.current_source_dir().split('/').get(-1)
LD += {
    'name' : sub_dir_name,
    'sources' : files([
                          'gsd.cc',
                          'decisionToSkipHit.cc'
                      ]),
    'headers' : files([
                          'gsd.h'
                      ]),
    'dependencies' : [geant4_deps, clhep_deps],
}
