sub_dir_name = meson.current_source_dir().split('/').get(-1)
sources = files('gsplash.cc')
headers = files('gsplash.h', )
qrc_examples_sources = files('examples/example.qrc')
examples = { 'test_gsplash' : [files('examples/example.cc'), ['-gui']] }

if get_option('i_test')
    LD += {
        'name' : sub_dir_name,
        'sources' : sources,
        'headers' : headers,
        'qrc_examples_sources' : qrc_examples_sources,
        'dependencies' : [yaml_cpp_dep, qt5_deps],
        'examples' : examples
    }
else
    LD += {
        'name' : sub_dir_name,
        'sources' : sources,
        'headers' : headers,
        'qrc_examples_sources' : qrc_examples_sources,
        'dependencies' : [yaml_cpp_dep, qt5_deps]
    }
endif
