sub_dir_name = meson.current_source_dir().split('/').get(-1)
LD += {
    'name' : sub_dir_name,
    'headers' : files(
        'glogger.h',
        'gbatch_session.h'
    ),
    'dependencies' : [clhep_deps, geant4_deps],
}
