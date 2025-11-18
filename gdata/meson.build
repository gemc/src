sub_dir_name = meson.current_source_dir().split('/').get(-1)

example_event_source = files('examples/gdata_event_example.cc')
example_frame_source = files('examples/gframe_event_example.cc')

verbosities = ['-verbosity.gevent_data=2',
               '-debug.gevent_data=true',
               '-verbosity.event_header=2',
               '-debug.event_header=true',
               '-verbosity.true_data=2',
               '-debug.true_data=true',
               '-verbosity.digitized_data=2',
               '-debug.digitized_data=true',
               '-verbosity.gtouchable=2',
               '-debug.gtouchable=true'
]


LD += {
    'name' : sub_dir_name,
    'sources' : files(
        'gDigitizedData.cc',
        'gTrueInfoData.cc',
        'event/gEventDataCollection.cc'
    ),
    'headers' : files(
        'gdataConventions.h',
        'gDigitizedData.h',
        'gDataCollection.h',
        'gTrueInfoData.h',
        'event/gEventHeader.h',
        'event/gEventDataCollection.h',
        'frame/gIntegralPayload.h',
        'frame/gFrameHeader.h',
        'frame/gFrameDataCollection.h'
    ),
    'additional_includes' : ['gdata', 'gdata/event'],
    'dependencies' : [yaml_cpp_dep, clhep_deps, geant4_deps],
    'examples' : {
        'test_gdata_event' : [example_event_source, ''],
        'test_gdata_event_verbose' : [example_event_source, verbosities],
        'test_gframe_event' : [example_frame_source, ''],
        'test_gframe_event_verbose' : [example_frame_source, verbosities]
    }
}
