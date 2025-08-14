sub_dir_name = meson.current_source_dir().split('/').get(-1)

verbosities = [
    '-verbosity.plugins=2',
    '-debug.plugins=true',
    '-verbosity.gdigitization=2',
    '-debug.gdigitization=true',
]

example_source = files('examples/plugin_load_example.cc')

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
        'test_gdynamic_plugin_load' : [example_source, ''],
        'test_gdynamic_plugin_load_verbose' : [example_source, verbosities],
    }
}

install_subdir(meson.current_source_dir() + '/dosimeterData', install_dir : '.')
