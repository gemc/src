sub_dir_name = meson.current_source_dir().split('/').get(-1)
LD += {
    'name' : sub_dir_name,
    'sources' : files([
                          'gphysics.cc',
                          'gphysicsOptions.cc'
                      ]),
    'headers' : files([
                          'gphysics.h',
                          'gphysicsOptions.h',
                          'gphysicsConventions.h'
                      ]),
    'dependencies' : [geant4_deps, clhep_deps],
    'examples' : {
        'gphysics_example' : [files(['examples/example.cc']), [meson.current_source_dir() + '/examples/example.jcard']],
    }
}
