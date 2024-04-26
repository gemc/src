sub_dir_name = meson.current_source_dir().split('/').get(-1)
LD += {
    'name' : sub_dir_name,
    'sources' : files([
                          'gemcUtilities.cc',
                          'loadDigitizationPlugins.cc'
                      ]),
    'headers' : files([
                          'gemcUtilities.h',
                      ]),
    'dependencies' : [geant4_deps, qt5_deps, clhep_deps],
}
