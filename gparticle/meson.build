sub_dir_name = meson.current_source_dir().split('/').get(-1)
single = '-sql=' + examples_dir + '/gemc.db'
gsystem = '-gsystem="[{name: b1}]"'
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
        'test_gparticle_single' : [files('examples/gparticle_example.cc'), [meson.current_source_dir() + '/examples/point_particle.yaml']],
        'test_gparticle_many' : [files('examples/gparticle_example.cc'), [meson.current_source_dir() + '/examples/point_particle.yaml']],
        'test_gparticle_many_verbosity' : [files('examples/gparticle_example.cc'), [meson.current_source_dir() + '/examples/point_particle.yaml']],
        'test_gparticle_many_debug' : [files('examples/gparticle_example.cc'), [meson.current_source_dir() + '/examples/point_particle.yaml']],
    }
}
