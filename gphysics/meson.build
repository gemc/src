sub_dir_name = meson.current_source_dir().split('/').get(-1)
LD += {
    'name' : sub_dir_name,
    'sources' : files(
                          'gphysics.cc',
                          'gphysicsOptions.cc'
                      ),
    'headers' : files(
                          'gphysics.h',
                          'gphysicsOptions.h',
                          'gphysicsConventions.h'
                      ),
    'dependencies' : [yaml_cpp_dep, clhep_deps, geant4_deps, ogl_deps, sqlite_dep, expat_dep],
    'examples' : {
        'test_gphysics' : [files('examples/example.cc'), [meson.current_source_dir() + '/examples/example.yaml']],
    }
}
