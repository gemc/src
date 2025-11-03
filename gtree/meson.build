sub_dir_name = meson.current_source_dir().split('/').get(-1)

with_gui = ['']
if get_option('i_test')
    with_gui = ['-gui']
endif

example_source = files('examples/gtree_example.cc')
examples_dir = get_option('prefix') + '/test/' + sub_dir_name
dbhost = ['-sql=' + examples_dir + '/gemc.db']
gsystem = ['-gsystem="[{name: b2}]"']

verbosities = ['-verbosity.gtree=2',
               '-debug.gtree=true']



sources = files(
    'gtree_options.cc',
    'gtree.cc'
)

headers = files(
    'gtree_options.h'
)

moc_headers = files(
    'gtree.h'
)

geo_build = {
    'test_dbselect_geo' : [files('examples/example.py')]
}

examples = {
    'test_gtree' : [example_source, with_gui + dbhost + gsystem],
    'test_gtree_verbose' : [example_source, with_gui + dbhost + gsystem + verbosities]
}

LD += {
    'name' : sub_dir_name,
    'sources' : sources,
    'headers' : headers,
    'moc_headers' : moc_headers,
    'dependencies' : [yaml_cpp_dep, qt6_deps, clhep_deps, geant4_deps, ogl_deps],
    'geo_build' : geo_build,
    'examples' : examples
}

