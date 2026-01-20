sub_dir_name = meson.current_source_dir().split('/').get(-1)
internal_deps = []

single = [ '-gparticle="[{name: e-, p: 2300, theta: 23.0}]"']
double = ['-gparticle="[{name: e-, p: 2300, theta: 23.0}, {name: proton, multiplicity: 2, p: 1200, theta: 14.0, delta_theta: 10}]"']

example_source = files('examples/gparticle_example.cc')

verbosities = ['-verbosity.gparticle=2',
               '-debug.gparticle=true'
]

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

    'dependencies' : [yaml_cpp_dep, clhep_deps, geant4_core_deps],
    'internal_dependencies' : internal_deps,

    'examples' : {
        'test_gparticle_single' : [example_source, single],
        'test_gparticle_double' : [example_source, double],
        'test_gparticle_single_verbose' : [example_source, single + verbosities],
        'test_gparticle_double_verbose' : [example_source, double + verbosities],
    }
}
