sub_dir_name = meson.current_source_dir().split('/').get(-1)
LD += {
    'name' : sub_dir_name,
    'sources' : files([
                          'gui.cc',
                          'leftButtons.cc',
                          'rightContent.cc',
                          'topLayout.cc'
                      ]),
    'moc_headers' : files([
                              'gui.h'
                          ]),
    'dependencies' : [geant4_deps, qt5_deps, clhep_deps],
}
