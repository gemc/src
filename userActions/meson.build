sub_dir_name = meson.current_source_dir().split('/').get(-1)
LD += {
    'name' : sub_dir_name,
    'sources' : files([
                          'gActionInitialization.cc',
                          'event/gEventAction.cc',
                          'event/gEventActionLog.cc',
                          'generator/gPrimaryGeneratorAction.cc',
                          'run/gRunAction.cc',
                          'run/gRun.cc'
                      ]),
    'headers' : files([
                          'gActionInitialization.h',
                          'event/gEventAction.h',
                          'generator/gPrimaryGeneratorAction.h',
                          'run/gRunAction.h',
                          'run/gRun.h'
                      ]),
    'dependencies' : [geant4_deps, qt5_deps, clhep_deps],
}
