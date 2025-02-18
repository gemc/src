sub_dir_name = meson.current_source_dir().split('/').get(-1)

sources = files(
    'g4dialog.cc',
    'g4dialogOptions.cc',
    'tabs/gcommands.cc',
)

headers = files(
    'g4dialog.h',
    'g4dialogOptions.h'
)

moc_headers = files(
    'tabs/gcommands.h'
)

examples = {
    'test_basic_dialog' : [files('examples/example.cc'), ['-gui']],
}

if get_option('i_test')
    LD += {
        'name' : sub_dir_name,
        'sources' : sources,
        'headers' : headers,
        'moc_headers' : moc_headers,
        'dependencies' : [yaml_cpp_dep, qt6_deps, clhep_deps, geant4_deps],
   #     'additional_includes' : ['g4dialog/tabs'],
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
