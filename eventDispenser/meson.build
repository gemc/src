sub_dir_name = meson.current_source_dir().split('/').get(-1)
LD += {
    'name' : sub_dir_name,
    'sources' : files([
                          'eventDispenser.cc',
                          'eventDispenserOptions.cc'
                      ]),
    'headers' : files([
                          'eventDispenser.h',
                          'eventDispenserOptions.h',
                          'eventDispenserConventions.h'
                      ]),
    'dependencies' : [geant4_deps, clhep_deps],
    'examples' : {
        'basic_dispenser' : [files(['examples/simple.cc']), ['']]
    }
}
