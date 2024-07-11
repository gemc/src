sub_dir_name = meson.current_source_dir().split('/').get(-1)
LD += {
    'name' : sub_dir_name,
    'sources' : files(
        'gQtButtonsWidget.cc',
        'gQtButtonsWidgetOptions.cc',
    ),
    'headers' : files(
        'gQtButtonsWidget.h',
        'gQtButtonsWidgetOptions.h',
    ),
    'moc_headers' : files(
        'gQtButtonsWidget.h'
    ),
    'qrc_examples_sources' : files(
        'examples/exampleResources.qrc'
    ),
    'dependencies' : [yaml_cpp_dep, qt5_deps],
    'examples' : {
        'test_gqtbuttons_widget' : [files('examples/example.cc'), ['']],
    }
}
