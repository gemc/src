sub_dir_name = meson.current_source_dir().split('/').get(-1)
internal_deps = ['goptions', 'guts', 'glogging', 'gfactory', 'gbase']

single = ['-gparticle="[{name: e-, p: 2300, theta: 23.0}]"']
double = ['-gparticle="[{name: e-, p: 2300, theta: 23.0}, {name: proton, multiplicity: 2, p: 1200, theta: 14.0, delta_theta: 10}]"']
lund_file = meson.current_source_dir() + '/examples/two_particles.lund'
dis_file = meson.current_source_dir() + '/examples/test_dis.dat'
lund = ['-gparticlefile="[{format: LUND, filename: ' + lund_file + '}]"']
dis = ['-gparticlefile="[{format: lund, filename: ' + dis_file + '}]"']
single_plus_lund = single + lund

example_source = files('examples/gparticle_example.cc')

verbosities = ['-verbosity.gparticle=2',
               '-debug.gparticle=true'
]

LD += {
    'name' : sub_dir_name,
    'sources' : files(
                          'gparticle.cc',
                          'gparticle_options.cc',
                          'gparticle_reader.cc',
                          'readers/gparticle_lund_reader.cc'
                      ),
    'headers' : files(
                          'gparticle.h',
                          'gparticle_options.h',
                          'gparticle_reader.h',
                          'gparticleConventions.h'
                      ),

    'dependencies' : [yaml_cpp_dep, clhep_deps, geant4_core_deps, expat_dep, zlib_dep],
    'internal_dependencies' : internal_deps,

    'examples' : {
        'test_gparticle_single' : [example_source, single],
        'test_gparticle_double' : [example_source, double],
        'test_gparticle_lund_file' : [example_source, lund],
        'test_gparticle_dis_lund_file' : [example_source, dis],
        'test_gparticle_single_plus_lund_file' : [example_source, single_plus_lund],
        'test_gparticle_single_verbose' : [example_source, single + verbosities],
        'test_gparticle_double_verbose' : [example_source, double + verbosities],
    }
}
