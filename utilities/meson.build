sub_dir_name = meson.current_source_dir().split('/').get(-1)
LD += {
    'name' : sub_dir_name,
    'sources' : files(
        'gemcUtilities.cc',
        'loadDigitizationPlugins.cc',
        'gemcOptions.cc',
    ),
    'headers' : files(
        'gemcUtilities.h',
        'gemcOptions.h',
    ),
    'dependencies' : [yaml_cpp_dep, clhep_deps, qt5_deps, geant4_deps],
}
