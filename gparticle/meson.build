sub_dir_name = meson.current_source_dir().split('/').get(-1)
LD += {
    'name' : sub_dir_name,
    'sources' : files(
                          'gparticle.cc',
                          'gparticle_options.cc'
                      ),
    'headers' : files(
                          'gparticle.h',
                          'gparticle_options.h',
                          'gparticleConventions.h'
                      ),
    'dependencies' : [yaml_cpp_dep, clhep_deps, geant4_deps],
    'examples' : {
        'test_gparticle_single' : [files('examples/example.cc'), [meson.current_source_dir() + '/examples/point_particle.yaml']],
    }
}
