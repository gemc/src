sub_dir_name = meson.current_source_dir().split('/').get(-1)
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
        'test_event_dispenser' : [files('examples/event_dispenser_example.cc'), ['-run_weights=' + meson.current_source_dir() + '/examples/runWeights.txt', '-n=1000', ]],
        'test_event_dispenser_with_file' : [files('examples/event_dispenser_example.cc'), ['-run_weights=' + meson.current_source_dir() + '/examples/runWeights.txt', '-n=1000']],
        'test_event_dispenser_with_file_verbose' : [files('examples/event_dispenser_example.cc'), ['-run_weights=' + meson.current_source_dir() + '/examples/runWeights.txt', '-n=1000', '-verbosity.eventdispenser=2', '-debug.eventdispenser=true']]
    }
}
