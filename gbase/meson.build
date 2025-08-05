sub_dir_name = meson.current_source_dir().split('/').get(-1)
example_source = files('examples/test_gbase.cc')
verbosity = ['-verbosity.g1=2', '-debug.g1=true']

LD += {
    'name' : sub_dir_name,
    'headers' : files(
        'gbase.h',
    ),
    'dependencies' : [yaml_cpp_dep, clhep_deps, geant4_deps],

    'examples' : {
        'test_gbase' : [example_source, verbosity],
    }
}
