sub_dir_name = meson.current_source_dir().split('/').get(-1)

example_source = files('examples/test_dbselect.cc')
examples_dir = get_option('prefix') + '/test/' + sub_dir_name
dbhost = ['-sql=' + examples_dir + '/gemc.db']
gsystem = ['-gsystem="[{name: b1}]"']

with_gui = ['']
if get_option('i_test')
    with_gui = ['-gui']
endif


sources = files(
    'dbselect_options.cc',
    'dbselectView.cc',
)

headers = files(
    'dbselect_options.h',
    'dbselectView.h',
)

moc_headers = files(
    'dbselectView.h',
)

qrc_examples_sources = files(
    'dbqt_resources.qrc'
)

geo_build = {
    'test_dbselect_geo' : [files('examples/example.py')]
}

examples = {
    'test_db_select' : [example_source, with_gui + dbhost + gsystem],
}

LD += {
    'name' : sub_dir_name,
    'sources' : sources,
    'headers' : headers,
    'moc_headers' : moc_headers,
    'qrc_examples_sources' : qrc_examples_sources,
    'dependencies' : [yaml_cpp_dep, qt6_deps, clhep_deps, geant4_deps, sqlite_dep],
    'geo_build' : geo_build,
    'examples' : examples
}

