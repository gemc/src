sub_dir_name = meson.current_source_dir().split('/').get(-1)
LD += {
    'name' : sub_dir_name,
    'sources' : files(
                          'gsd.cc'
                      ),
    'headers' : files(
                          'gsd.h'
                      ),
    'dependencies' : [yaml_cpp_dep, clhep_deps, geant4_deps],
}
