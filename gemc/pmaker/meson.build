sub_dir_name = meson.current_source_dir().split('/').get(-1)
internal_deps = ['goptions', 'guts', 'glogging', 'gbase', 'gparticle']

LD += {
    'name' : sub_dir_name,
    'sources' : files(
        'pmaker_options.cc',
        'pmakerView.cc',
        'pmakerTab.cc',
    ),
    'headers' : files(
        'pmaker_options.h',
        'pmakerView.h',
        'pmakerTab.h',
    ),
    'moc_headers' : files(
        'pmakerView.h',
        'pmakerTab.h',
    ),
    'dependencies' : [yaml_cpp_dep, qt6_deps, clhep_deps, geant4_core_deps],
    'internal_dependencies' : internal_deps,
}
