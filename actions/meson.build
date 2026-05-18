sub_dir_name = meson.current_source_dir().split('/').get(-1)
internal_deps = ['goptions', 'guts', 'glogging', 'gbase', 'gfactory', 'gparticle']

dis_file = meson.project_source_root() + '/gparticle/examples/test_dis.dat'
generator_file_events = [
    '-gparticlefile="[{format: lund, filename: ' + dis_file + '}]"',
    '-gparticle="[{name: e-, p: 2300, theta: 23.0}]"',
    '-verbosity.generator=2'
]

LD += {
    'name' : sub_dir_name,
    'sources' : files(
        'gaction.cc',
        'event/gEventAction.cc',
        'generator/gPrimaryGeneratorAction.cc',
        'run/gRunAction.cc',
        'run/gRun.cc'
    ),
    'headers' : files(
        'gaction.h',
        'event/gEventAction.h',
        'gactionConventions.h',
        'generator/gPrimaryGeneratorAction.h',
        'run/gRunAction.h',
        'run/gRun.h'
    ),
    'dependencies' : [yaml_cpp_dep, clhep_deps, qt6_deps, geant4_core_deps],
    'internal_dependencies' : internal_deps,
    'examples' : {
        'test_generator_lund_file_events' : [
            files('examples/generator_file_events_example.cc'),
            generator_file_events
        ],
    }
}
