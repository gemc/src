sub_dir_name = meson.current_source_dir().split('/').get(-1)
LD += {
    'name' : sub_dir_name,
    'sources' : files(
        'gdata_options.cc',
        'event/gDigitizedData.cc',
        'event/gTrueInfoData.cc',
        'event/gEventDataCollection.cc'
    ),
    'headers' : files(
        'gdata_options.h',
        'gdataConventions.h',
        'event/gDigitizedData.h',
        'event/gTrueInfoData.h',
        'event/gDataCollection.h',
        'event/gEventHeader.h',
        'event/gEventDataCollection.h',
        'frame/gIntegralPayload.h',
        'frame/gFrameHeader.h',
        'frame/gFrameDataCollection.h'
    ),
    'additional_includes' : ['gdata', 'gdata/event'],
    'dependencies' : [yaml_cpp_dep, clhep_deps, geant4_deps],
    'examples' : {
        'test_gdata_event' : [files('examples/gdata_event_example.cc'), ''],
        'test_gdata_event_verbose' : [files('examples/gdata_event_example.cc'), '-verbosity.gdata=2', '-verbosity.gtouchable=2', '-debug.gdata=true', '-debug.gtouchable=true'],
        'test_gframe_event' : [files('examples/gframe_event_example.cc'), ''],
        'test_gframe_event_verbose' : [files('examples/gframe_event_example.cc'), '-verbosity.gdata=2', '-verbosity.gtouchable=2', '-debug.gdata=true', '-debug.gtouchable=true']
    }
}
