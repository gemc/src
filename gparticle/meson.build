# expat needed because g4processes
sub_dir_name = meson.current_source_dir().split('/').get(-1)
single = '-gparticle="[{name: e-, p: 2300, theta: 23.0}]"'
double = '-gparticle="[{name: e-, p: 2300, theta: 23.0}, {name: proton, multiplicity: 2, p: 1200, theta: 14.0, delta_theta: 10}]"'
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
    'dependencies' : [yaml_cpp_dep, clhep_deps, geant4_deps, expat_dep],
    'examples' : {
        'test_gparticle_single' : [files('examples/gparticle_example.cc'), [single]],
        'test_gparticle_double' : [files('examples/gparticle_example.cc'),[double]],
        'test_gparticle_double_verbose' : [files('examples/gparticle_example.cc'), [double, '-verbosity.gparticle=2', '-debug.gparticle=true']],
    }
}
