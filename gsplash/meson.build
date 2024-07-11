sub_dir_name = meson.current_source_dir().split('/').get(-1)
LD += {
    'name' : sub_dir_name,
    'sources' : files(
                          'gsplash.cc'
                      ),
    'headers' : files(
                          'gsplash.h',
                      ),
    'qrc_examples_sources' : files(
                              'examples/example.qrc'
                          ),
    'dependencies' : [yaml_cpp_dep, qt5_deps],
    'examples' : {
        'test_gsplash' : [files('examples/example.cc'), ['gui']],
    }
}
