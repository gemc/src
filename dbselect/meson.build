sub_dir_name = meson.current_source_dir().split('/').get(-1)

sources = files(
    'dbselect.cc',
)

headers = files(
    'dbselect.h',
)

moc_headers = files(
    'dbselect.h',
)


qrc_examples_sources = files(
    'qt_resources.qrc'
)

examples = {
    'test_db_select' : [files('examples/example.cc'), ['-gui']],
}

if get_option('i_test')
    LD += {
        'name' : sub_dir_name,
        'sources' : sources,
        'headers' : headers,
        'moc_headers' : moc_headers,
        'qrc_examples_sources' : qrc_examples_sources,
        'dependencies' : [yaml_cpp_dep, qt6_deps, sqlite_dep],
        'examples' : examples
    }
else
    LD += {
        'name' : sub_dir_name,
        'sources' : sources,
        'headers' : headers,
        'moc_headers' : moc_headers,
        'dependencies' : [yaml_cpp_dep, qt6_deps, sqlite_dep],
    }
endif
