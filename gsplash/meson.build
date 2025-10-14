sub_dir_name = meson.current_source_dir().split('/').get(-1)

with_gui = ['']
if get_option('i_test')
    with_gui = ['-gui']
endif

example_source = files('examples/gsplash_example.cc')

verbosities = [
    '-verbosity.gsplash=2',
    '-debug.gsplash=true']

sources = files(
    'gsplash.cc'
)

headers = files(
    'gsplash.h'
)

qrc_examples_sources = files(
    'examples/qt_resources_example.qrc'
)

examples = {
    'test_gsplash' : [example_source, with_gui],
    'test_gsplash_verbose' : [example_source, with_gui + verbosities]
}


LD += {
    'name' : sub_dir_name,
    'sources' : sources,
    'headers' : headers,
    'qrc_examples_sources' : qrc_examples_sources,
    'dependencies' : [yaml_cpp_dep, qt6_deps, geant4_deps],
    'examples' : examples
}
