sub_dir_name = meson.current_source_dir().split('/').get(-1)
LD += {
    'name' : sub_dir_name,
    'sources' : files([
                          'event/gDigitizedData.cc',
                          'event/gTrueInfoData.cc',
                          'event/gDataCollection.cc',
                          'event/gEventDataCollectionHeader.cc',
                          'event/gEventDataCollection.cc',
                          'frame/gIntegralPayload.cc',
                          'frame/gFrameDataCollectionHeader.cc',
                          'frame/gFrameDataCollection.cc'
                      ]),
    'headers' : files([
                          'gdataConventions.h',
                          'event/gDigitizedData.h',
                          'event/gTrueInfoData.h',
                          'event/gDataCollection.h',
                          'event/gEventDataCollectionHeader.h',
                          'event/gEventDataCollection.h',
                          'frame/gIntegralPayload.h',
                          'frame/gFrameDataCollectionHeader.h',
                          'frame/gFrameDataCollection.h'
                      ]),
    'additional_includes' : ['gdata/event'],
    'dependencies' : [geant4_deps, clhep_deps],
    'examples' : {
        'gdata_event' : [files(['examples/event_data.cc']), ['']],
    }
}
