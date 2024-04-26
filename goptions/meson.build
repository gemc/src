sub_dir_name = meson.current_source_dir().split('/').get(-1)
run_command('version.py', check: true)
LD += {
    'name' : sub_dir_name,
    'sources' : files([
                          'goption.cc',
                          'goptions.cc'
                      ]),
    'headers' : files([
                          'json.hpp',
                          'goption.h',
                          'goptions.h',
                          'gversion.h',
                          'goptionsConventions.h'
                      ]),
    'examples' : {
        'goption_basic' : [files(['examples/simple.cc']), [meson.current_source_dir() + '/examples/simple.jcard']],
        'goption_structured' : [files(['examples/structured.cc']), [meson.current_source_dir() + '/examples/structured.jcard']]
    }
}
