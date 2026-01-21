sub_dir_name = meson.current_source_dir().split('/').get(-1)
#internal_deps = ['goptions', 'guts', 'glogging', 'g4system', 'gsystem']
internal_deps = ['goptions', 'guts', 'glogging', 'gtouchable', 'gdata', 'gtranslationTable', 'ghit', 'gfactory', 'gdynamicDigitization',  'g4system', 'gsystem', 'gdetector', 'gstreamer']

with_gui = ['']
if get_option('i_test')
    with_gui = ['-gui']
endif

example_source = files('examples/gtree_example.cc')
verbosities = ['-verbosity.gtree=2',
               '-debug.gtree=true']

examples_dir = get_option('prefix') + '/test/' + sub_dir_name
dbhost = ['-sql=' + examples_dir + '/gemc.db']
gsystem = ['-gsystem="[{name: b2}]"']

sources = files(
    'gtree_options.cc',
    'gtree.cc',
    'right_widget.cc'
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

qrc_examples_sources = files(
    'images/qtresources.qrc'
)

examples = {
    'test_gtree' : [example_source, with_gui + dbhost + gsystem],
    'test_gtree_verbose' : [example_source, with_gui + dbhost + gsystem + verbosities]
}

LD += {
    'name' : sub_dir_name,
    'sources' : sources,
    'headers' : headers,
    'moc_headers' : moc_headers,
    'qrc_examples_sources' : qrc_examples_sources,
    'dependencies' : [yaml_cpp_dep, qt6_deps, clhep_deps, geant4_deps, ogl_deps],
    'internal_dependencies' : internal_deps,
    'geo_build' : geo_build,
    'examples' : examples
}

