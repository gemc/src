sub_dir_name = meson.current_source_dir().split('/').get(-1)

with_gui = ['']
if get_option('i_test')
    with_gui = ['-gui']
endif

example_source = files('examples/gboard_example.cc')

verbosities = [
    '-verbosity.gboard=2',
    '-debug.gboard=true']

sources = files(
    'gboard.cc',
    'gui_session.cc',
)

headers = files(
    'gboard.h',
    'gui_session.h',
)

moc_headers = files(
    'gboard.h',
)

examples = {
    'test_gboard' : [example_source, with_gui],
    'test_gboard_verbose' : [example_source, with_gui + verbosities]
}


if get_option('i_test')
    LD += {
        'name' : sub_dir_name,
        'sources' : sources,
        'headers' : headers,
        'moc_headers' : moc_headers,
        'dependencies' : [yaml_cpp_dep, qt6_deps, clhep_deps, geant4_deps],
        'additional_includes' : ['gboard'],
        'examples' : examples
    }
else
    LD += {
        'name' : sub_dir_name,
        'sources' : sources,
        'headers' : headers,
        'moc_headers' : moc_headers,
        'dependencies' : [yaml_cpp_dep, qt6_deps, clhep_deps, geant4_deps],
        'additional_includes' : ['gboard'],
    }
endif
