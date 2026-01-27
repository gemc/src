sub_dir_name = meson.current_source_dir().split('/').get(-1)
internal_deps = ['goptions', 'guts', 'glogging']

with_gui = ['']
if get_option('i_test')
    with_gui = ['-gui']
endif

sources = files(
    'gQtButtonsWidget.cc',
)

headers = files(
    'gQtButtonsWidget.h',
)

moc_headers = files(
    'gQtButtonsWidget.h'
)

qrc_examples_sources = files(
    'examples/exampleResources.qrc'
)

examples = {
    'test_gqtbuttons_widget' : [files('examples/gqtbuttons_example.cc'), with_gui],
}

LD += {
    'name' : sub_dir_name,
    'sources' : sources,
    'headers' : headers,
    'moc_headers' : moc_headers,
    'qrc_examples_sources' : qrc_examples_sources,
    'dependencies' : [yaml_cpp_dep, qt6_deps],
    'internal_dependencies' : internal_deps,
    'examples' : examples
}

