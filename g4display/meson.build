sub_dir_name = meson.current_source_dir().split('/').get(-1)

with_gui = ['']
if get_option('i_test')
    with_gui = ['-gui']
endif

example_source = files('examples/g4display_example.cc')

verbosities = ['-verbosity.g4display=2',
               '-debug.g4display=true',
               '-verbosity.g4scene=2',
               '-debug.g4scene=true']

yaml = [meson.current_source_dir() + '/examples/example.yaml']

sources = files(
    'g4display_options.cc',
    'g4SceneProperties.cc',
    'g4Text.cc',
    'g4display.cc',
    'tabs/g4displayview.cc',
    'tabs/g4displayutilities.cc'
)

headers = files(
    'g4display_options.h',
    'g4SceneProperties.h',
    'g4Text.h',
    'g4display.h',
    'tabs/g4displayview.h',
    'tabs/g4displayutilities.h'
)

moc_headers = files(
    'tabs/g4displayview.h',
    'tabs/g4displayutilities.h'
)

qrc_examples_sources = files(
    'qt_resources.qrc'
)

examples = {
    'test_g4display' : [example_source, yaml + with_gui],
    'test_g4display_verbose' : [example_source, yaml + with_gui + verbosities]
}

LD += {
    'name' : sub_dir_name,
    'sources' : sources,
    'headers' : headers,
    'moc_headers' : moc_headers,
    'qrc_examples_sources' : qrc_examples_sources,
    'dependencies' : [yaml_cpp_dep, qt6_deps, clhep_deps, geant4_deps, ogl_deps, sqlite_dep],
    'additional_includes' : ['g4display', 'g4display/tabs'],
    'examples' : examples
}

