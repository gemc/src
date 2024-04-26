sub_dir_name = meson.current_source_dir().split('/').get(-1)
LD += {
    'name' : sub_dir_name,
    'sources' : files([
                          'gdetectorConstruction.cc'
                      ]),
    'headers' : files([
                          'gdetectorConstruction.h'
                      ]),
    'dependencies' : [geant4_deps, clhep_deps],
}
