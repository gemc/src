sub_dir_name = meson.current_source_dir().split('/').get(-1)

sources = files(
    'gQtButtonsWidget.cc',
    'gQtButtonsWidgetOptions.cc',
)

headers = files(
    'gQtButtonsWidget.h',
    'gQtButtonsWidgetOptions.h',
)

moc_headers = files(
    'gQtButtonsWidget.h'
)

qrc_examples_sources = files(
    'examples/exampleResources.qrc'
)

examples = {
    'test_gqtbuttons_widget' : [files('examples/example.cc'), ['-gui']],
}

if get_option('i_test')
    LD += {
        'name' : sub_dir_name,
        'sources' : sources,
        'headers' : headers,
        'moc_headers' : moc_headers,
        'qrc_examples_sources' : qrc_examples_sources,
        'dependencies' : [yaml_cpp_dep, qt6_deps],
        'examples' : examples
    }
else
    LD += {
        'name' : sub_dir_name,
        'sources' : sources,
        'headers' : headers,
        'moc_headers' : moc_headers,
        'qrc_examples_sources' : qr_examples_sources,
        'dependencies' : [yaml_cpp_dep, qt6_deps]
    }
endif
