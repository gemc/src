sub_dir_name = meson.current_source_dir().split('/').get(-1)
LD += {
    'name' : sub_dir_name,
    'sources' : files([
                          'gparticle.cc',
                          'gparticleOptions.cc'
                      ]),
    'headers' : files([
                          'gparticle.h',
                          'gparticleOptions.h',
                          'gparticleConventions.h'
                      ]),
    'dependencies' : [geant4_deps, clhep_deps],
    'examples' : {
        'gparticle_example' : [files(['examples/example.cc']), [meson.current_source_dir() + '/examples/pointParticle.jcard']],
    }
}
