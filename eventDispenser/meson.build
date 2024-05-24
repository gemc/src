sub_dir_name = meson.current_source_dir().split('/').get(-1)
LD += {
    'name' : sub_dir_name,
    'sources' : files(
        'eventDispenser.cc',
        'eventDispenserOptions.cc'
    ),
    'headers' : files(
        'eventDispenser.h',
        'eventDispenserOptions.h',
        'eventDispenserConventions.h'
    ),
    'dependencies' : [yaml_cpp_dep, clhep_deps, geant4_deps],
    'examples' : {
        'basic_event_dispenser' : [files('examples/simple.cc'), meson.current_source_dir() + '/examples/simple.yaml' ]
    }
}
