sub_dir_name = meson.current_source_dir().split('/').get(-1)
internal_deps = ['goptions', 'guts']

install_subdir('../../translations',
               install_dir: get_option('datadir') / 'gemc/translations/gemc',
               strip_directory: true,
               exclude_files: ['catalog.yaml', 'README.md'],
               exclude_directories: ['gui'],
               install_tag: 'translations')

LD += {
    'name' : sub_dir_name,
    'sources' : files('glogger.cc'),
    'additional_includes' : ['gemc/glogging'],
    'headers' : files(
        'glogger.h',
        'gmessageCatalog.h',
        'gbatch_session.h'
    ),
    'dependencies' : [yaml_cpp_dep, clhep_deps, geant4_core_deps],
    'internal_dependencies' : internal_deps,
}
