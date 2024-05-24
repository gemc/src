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
        'define_options' : [files('examples/define_options.cc'), ''],
        'define_options_help' : [files('examples/define_options.cc'), '-h'],
        'parse_file' : [files('examples/define_options.cc'), meson.current_source_dir() + '/examples/simple.yaml'],
    }
}
