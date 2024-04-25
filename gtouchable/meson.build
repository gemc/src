sub_dir_name = meson.current_source_dir().split('/').get(-1)
LD += {
    'name' : sub_dir_name,
    'sources' : files([
                          'gelectronic.cc',
                          'gtouchable.cc',

                      ]),
    'headers' : files([
                          'gelectronic.h',
                          'gtouchable.h',
                          'gtouchableConventions.h'
                      ]),
    'examples' : {
        'basic_gtouchable' : [files(['examples/simple.cc']), ['']]
    }
}
