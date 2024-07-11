sub_dir_name = meson.current_source_dir().split('/').get(-1)
run_command('version.py', check : true)
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
    'dependencies' : [yaml_cpp_dep],

    'examples' : {
        'test_goptions_definitions' : [files('examples/define_options.cc'), ''],
        'test_goptions_help' : [files('examples/define_options.cc'), '-h'],
        'test_goptions_parse_file' : [files('examples/define_options.cc'), meson.current_source_dir() + '/examples/simple.yaml'],
    }
}
