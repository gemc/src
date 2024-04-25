sub_dir_name = meson.current_source_dir().split('/').get(-1)
LD += {
    'name' : sub_dir_name,
    'sources' : files([
                          'gdynamicdigitization.cc',
                          'gFluxDigitization.cc',
                          'gParticleCounterDigitization.cc',
                          'gDosimeterDigitization.cc',

                      ]),
    'headers' : files([
                          'ginternalDigitization.h',
                          'greadoutSpecs.h'
                      ]),
    'dependencies' : [geant4_deps, clhep_deps],
    'plugins' : {
        'CTofRoutineExample' : [files(['examples/ctofRoutinesExample.cc']), false],
    },
    'examples' : {
        'gdynamic_plugin_load_example' : [files(['examples/plugin_load_example.cc']), ['']]
    }
}
