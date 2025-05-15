sub_dir_name = meson.current_source_dir().split('/').get(-1)
LD += {
    'name' : sub_dir_name,
    'sources' : files(
        'gdynamicdigitization.cc',
        'gFluxDigitization.cc',
        'gParticleCounterDigitization.cc',
        'gDosimeterDigitization.cc',
        'gdynamicdigitization_options.cc'
    ),
    'headers' : files(
        'gdynamicdigitization_options.h',
        'ginternalDigitization.h',
        'greadoutSpecs.h'
    ),
    'dependencies' : [yaml_cpp_dep, clhep_deps, geant4_deps],
    'plugins' : {
        'test_gdynamic_plugin' : [files('examples/gplugin_test_example.cc'), true],
    },
    'examples' : {
        'test_gdynamic_plugin_load' : [files('examples/plugin_load_example.cc'), ''],
        'test_gdynamic_plugin_load_verbose' : [files('examples/plugin_load_example.cc'),
                                                 '-verbosity.gdata=2', '-verbosity.plugins=2', '-verbosity.gtranslationTable=2', '-verbosity.gdigitization=2',
                                                 '-debug.gdata=true', '-debug.plugins=true', '-debug.gtranslationTable=true', '-debug.gdigitization=true'],
    }
}

install_subdir(meson.current_source_dir() + '/dosimeterData', install_dir : '.')
