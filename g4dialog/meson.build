sub_dir_name = meson.current_source_dir().split('/').get(-1)

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
    'test_g4dialog' : [files('examples/g4dialog_example.cc'), ['-gui']],
    'test_g4dialog_debug' : [files('examples/g4dialog_example.cc'), ['-gui', '-debug.g4dialog=true', '-verbosity.g4dialog=2']],
}


if get_option('i_test')
    LD += {
        'name' : sub_dir_name,
        'sources' : sources,
        'headers' : headers,
        'moc_headers' : moc_headers,
        'dependencies' : [yaml_cpp_dep, qt6_deps, clhep_deps, geant4_deps],
        'additional_includes' : ['g4dialog/tabs'],
        'examples' : examples
    }
else
    LD += {
        'name' : sub_dir_name,
        'sources' : sources,
        'headers' : headers,
        'moc_headers' : moc_headers,
        'dependencies' : [yaml_cpp_dep, qt6_deps, clhep_deps, geant4_deps],
        'additional_includes' : ['g4dialog/tabs']
    }
endif
