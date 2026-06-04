sub_dir_name = meson.current_source_dir().split('/').get(-1)
internal_deps = ['guts']

run_command('version.py', check : true)
example_source = files('examples/define_options.cc')


LD += {
    'name' : sub_dir_name,
    'sources' : files(
        'goption.cc',
        'goptions.cc'
    ),
    'headers' : files(
        'gswitch.h',
        'goption.h',
        'goptions.h',
        'gversion.h',
        'goptionsConventions.h'
    ),
    'dependencies' : [yaml_cpp_dep, clhep_deps, geant4_core_deps],
    'internal_dependencies' : internal_deps,

    'examples' : {
        'test_goptions_definitions' : [example_source, ''],
        'test_goptions_help' : [example_source, '-h'],
        'test_goptions_parse_file' : [example_source, meson.current_source_dir() + '/examples/simple.yaml'],
    }
}
