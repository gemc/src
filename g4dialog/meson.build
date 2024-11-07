sub_dir_name = meson.current_source_dir().split('/').get(-1)

sources = files(
    'g4dialog.cc'
)

headers = files(
    'g4dialog.h'
)

#moc_headers = files(
#    'tabs/g4displaycamera.h',
#    'tabs/g4displayslice.h'
#)

examples = {
    'test_basic_dialog' : [files('examples/example.cc'), meson.current_source_dir() + '/examples/example.yaml'],
}

if get_option('i_test')
    LD += {
        'name' : sub_dir_name,
        'sources' : sources,
        'headers' : headers,
        'moc_headers' : moc_headers,
        'dependencies' : [yaml_cpp_dep, qt5_deps, clhep_deps, geant4_deps],
   #     'additional_includes' : ['g4dialog/tabs'],
        'examples' : examples
    }
else
    LD += {
        'name' : sub_dir_name,
        'sources' : sources,
        'headers' : headers,
        'moc_headers' : moc_headers,
        'dependencies' : [yaml_cpp_dep, qt5_deps, clhep_deps, geant4_deps],
  #      'additional_includes' : ['g4dialog/tabs']
    }
endif
