sub_dir_name = meson.current_source_dir().split('/').get(-1)
internal_deps = ['gaction', 'ganalysis', 'gphysics']

LD += {
    'name' : sub_dir_name,
    'sources' : files(
        'gui.cc',
        'physicsView.cc',
        'leftButtons.cc',
        'rightContent.cc',
        'topLayout.cc'
    ),
    'moc_headers' : files(
        'gui.h',
        'physicsView.h'
    ),
    'dependencies' : [yaml_cpp_dep, clhep_deps, qt6_deps, geant4_deps],
    'internal_dependencies' : internal_deps,
}
