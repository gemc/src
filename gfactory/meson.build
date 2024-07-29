sub_dir_name = meson.current_source_dir().split('/').get(-1)
LD += {
    'name' : sub_dir_name,
    'headers' : files(
                          'gdl.h',
                          'gfactory.h'
                      ),
    'plugins' : {
        'test_dyn_factory1' : [files('examples/FordFactory.cc'), true],
        'test_dyn_factory2' : [files('examples/TeslaFactory.cc'), true],
    },
    'examples' : {
        'test_gfactory_static_and_dynamic_load' : [files('examples/static_and_dynamic_example.cc', 'examples/ShapeFactory.cc'), '']
    }
}
