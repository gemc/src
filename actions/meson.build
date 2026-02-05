sub_dir_name = meson.current_source_dir().split('/').get(-1)
internal_deps = []

LD += {
    'name' : sub_dir_name,
    'sources' : files(
        'gaction.cc',
        'event/gEventAction.cc',
        'generator/gPrimaryGeneratorAction.cc',
        'run/gRunAction.cc',
        'run/gRun.cc'
    ),
    'headers' : files(
        'gaction.h',
        'event/gEventAction.h',
        'gactionConventions.h',
        'generator/gPrimaryGeneratorAction.h',
        'run/gRunAction.h',
        'run/gRun.h'
    ),
    'dependencies' : [yaml_cpp_dep, clhep_deps, qt6_deps, geant4_core_deps],
    'internal_dependencies' : internal_deps,
}
