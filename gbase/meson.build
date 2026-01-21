sub_dir_name = meson.current_source_dir().split('/').get(-1)
internal_deps = ['glogging', 'goptions', 'guts']

example_source = files('examples/test_gbase.cc')
verbosity = [
    '-verbosity.hello1=2',
    '-debug.hello1=true',
    '-verbosity.hello2=2',
    '-debug.hello2=true'
]

LD += {
    'name' : sub_dir_name,
    'headers' : files(
        'gbase.h',
    ),
    'dependencies' : [yaml_cpp_dep, clhep_deps, geant4_core_deps],
    'internal_dependencies' : internal_deps,

    'examples' : {
        'test_gbase' : [example_source, verbosity],
    }
}
