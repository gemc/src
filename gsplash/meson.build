sub_dir_name = meson.current_source_dir().split('/').get(-1)
LD += {
    'name' : sub_dir_name,
    'sources' : files([
                          'gsplash.cc'
                      ]),
    'headers' : files([
                          'gsplash.h',
                      ]),
    'qrc_examples_sources' : files([
                              'examples/example.qrc'
                          ]),
    'dependencies' : [qt5_deps],
    'examples' : {
        'gsplash_example' : [files(['examples/example.cc']), ['gui']],
    }
}
