sub_dir_name = meson.current_source_dir().split('/').get(-1)

with_gui = ['']
if get_option('i_test')
    with_gui = ['-gui']
endif

example_source = files('examples/g4dialog_example.cc')

verbosities = [
    '-verbosity.g4dialog=2',
    '-debug.g4dialog=true',
    '-verbosity.gboard=2',
    '-debug.gboard=true']

sources = files(
    'g4dialog.cc',
    'g4dialog_options.cc',
    'gui_session.cc',
    'tabs/gcommands.cc',
    'tabs/gboard.cc',
)

headers = files(
    'g4dialog.h',
    'g4dialog_options.h',
    'gui_session.h',
)

moc_headers = files(
    'tabs/gcommands.h',
    'tabs/gboard.h',
)

examples = {
    'test_g4dialog' : [example_source, with_gui],
    'test_g4dialog_verbose' : [example_source, with_gui + verbosities]
}


if get_option('i_test')
    LD += {
        'name' : sub_dir_name,
        'sources' : sources,
        'headers' : headers,
        'moc_headers' : moc_headers,
        'dependencies' : [yaml_cpp_dep, qt6_deps, clhep_deps, geant4_deps],
        'additional_includes' : ['g4dialog', 'g4dialog/tabs'],
        'examples' : examples
    }
else
    LD += {
        'name' : sub_dir_name,
        'sources' : sources,
        'headers' : headers,
        'moc_headers' : moc_headers,
        'dependencies' : [yaml_cpp_dep, qt6_deps, clhep_deps, geant4_deps],
        'additional_includes' : ['g4dialog', 'g4dialog/tabs'],
    }
endif
