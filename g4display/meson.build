sub_dir_name = meson.current_source_dir().split('/').get(-1)

sources = files(
    'g4displayOptions.cc',
    'g4SceneProperties.cc',
    'g4Text.cc',
    'g4display.cc',
    'tabs/g4displayview.cc',
    'tabs/g4displayutilities.cc'
)

headers = files(
    'g4displayOptions.h',
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
    'test_basic_display' : [files('examples/example.cc'), [meson.current_source_dir() + '/examples/example.yaml', '-gui']],
}

if get_option('i_test')
    LD += {
        'name' : sub_dir_name,
        'sources' : sources,
        'headers' : headers,
        'moc_headers' : moc_headers,
        'qrc_examples_sources' : qrc_examples_sources,
        'dependencies' : [yaml_cpp_dep, qt6_deps, clhep_deps, geant4_deps, ogl_deps, sqlite_dep],
        'additional_includes' : ['g4display/tabs'],
        'examples' : examples
    }
else
    LD += {
        'name' : sub_dir_name,
        'sources' : sources,
        'headers' : headers,
        'moc_headers' : moc_headers,
        'dependencies' : [yaml_cpp_dep, qt6_deps, clhep_deps, geant4_deps, ogl_deps, sqlite_dep],
        'additional_includes' : ['g4display/tabs']
    }
endif
