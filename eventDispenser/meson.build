sub_dir_name = meson.current_source_dir().split('/').get(-1)
verbosities = ['-verbosity.eventdispenser=2',
               '-debug.eventdispenser=true'
]
weight_file_option = '-run_weights=' + meson.current_source_dir() + '/examples/runWeights.txt'

LD += {
    'name' : sub_dir_name,
    'sources' : files(
        'eventDispenser.cc',
        'eventDispenser_options.cc'
    ),
    'headers' : files(
        'eventDispenser.h',
        'eventDispenser_options.h',
        'eventDispenserConventions.h'
    ),
    'dependencies' : [yaml_cpp_dep, clhep_deps, geant4_deps],
    'examples' : {
        'test_event_dispenser' : [files('examples/event_dispenser_example.cc'), [weight_file_option, '-n=1000', ]],
        'test_event_dispenser_verbose' : [files('examples/event_dispenser_example.cc'), [weight_file_option, '-n=1000'] + verbosities]
    }
}
