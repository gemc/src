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
        'event/gEventDataCollectionHeader.h',
        'event/gEventDataCollection.h',
        'frame/gIntegralPayload.h',
        'frame/gFrameDataCollectionHeader.h',
        'frame/gFrameDataCollection.h'
    ),
    'additional_includes' : ['gdata/event'],
    'dependencies' : [yaml_cpp_dep, clhep_deps, geant4_deps],
    'examples' : {
        'test_gdata_event' : [files('examples/gdata_event_example.cc'), ''],
        'test_gdata_event_debug' : [files('examples/gdata_event_example.cc'), '-debug.gdata=1', '-debug.gtouchable=1'],
        'test_gdata_event_verbosity' : [files('examples/gdata_event_example.cc'), '-verbosity.gdata=1', '-verbosity.gtouchable=1'],
        'test_gframe_event' : [files('examples/gframe_event_example.cc'), ''],
        'test_gframe_event_debug' : [files('examples/gframe_event_example.cc'), '-debug.gdata=1', '-debug.gtouchable=1'],
        'test_gframe_event_verbosity' : [files('examples/gframe_event_example.cc'), '-verbosity.gdata=1', '-verbosity.gtouchable=1']
    }
}
