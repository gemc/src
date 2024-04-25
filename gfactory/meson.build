sub_dir_name = meson.current_source_dir().split('/').get(-1)
LD += {
    'name' : sub_dir_name,
    'headers' : files([
                          'gdl.h',
                          'gfactory.h'
                      ]),
    'plugins' : {
        'fordFactory' : [files(['examples/FordFactory.cc']), false],
        'teslaFactory' : [files(['examples/TeslaFactory.cc']), false],
    },
    'examples' : {
        'gfactory_static_and_dynamic_example' : [files(['examples/static_and_dynamic_example.cc', 'examples/ShapeFactory.cc']), '']
    }
}
