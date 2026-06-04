sub_dir_name = meson.current_source_dir().split('/').get(-1)
internal_deps = []

LD += {
    'name' : sub_dir_name,
    'sources' : files(
        'gui.cc',
        'leftButtons.cc',
        'rightContent.cc',
        'topLayout.cc'
    ),
    'moc_headers' : files(
        'gui.h'
    ),
    'dependencies' : [yaml_cpp_dep, clhep_deps, qt6_deps, geant4_deps],
    'internal_dependencies' : internal_deps,
}
