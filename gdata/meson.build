sub_dir_name = meson.current_source_dir().split('/').get(-1)
internal_deps = ['goptions', 'guts', 'gtouchable', 'ghit']

example_event_source = files('examples/event_example.cc')
example_frame_source = files('examples/gframe_example.cc')
example_run_source = files('examples/run_example.cc')

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

verbosities_run = ['-verbosity.grun_data=2',
                   '-debug.grun_data=true',
]


LD += {
    'name' : sub_dir_name,
    'sources' : files(
        'gDigitizedData.cc',
        'gTrueInfoData.cc',
        'run/gRunDataCollection.cc',
        'event/gEventDataCollection.cc'
    ),
    'headers' : files(
        'gdataConventions.h',
        'gDigitizedData.h',
        'gDataCollection.h',
        'gTrueInfoData.h',
        'run/gRunHeader.h',
        'run/gRunDataCollection.h',
        'event/gEventHeader.h',
        'event/gEventDataCollection.h',
        'frame/gIntegralPayload.h',
        'frame/gFrameHeader.h',
        'frame/gFrameDataCollection.h'
    ),
    'additional_includes' : ['gdata', 'gdata/event', 'gdata/run'],

    'dependencies' : [yaml_cpp_dep, clhep_deps, geant4_core_deps],
    'internal_dependencies' : internal_deps,

    'examples' : {
        'test_gdata_event' : [example_event_source, ''],
        'test_gdata_event_verbose' : [example_event_source, verbosities],
        'test_gdata_gframe' : [example_frame_source, ''],
        'test_gdata_gframe_verbose' : [example_frame_source, verbosities],
        'test_gdata_run_event' : [example_run_source, ''],
        'test_gdata_run_verbose' : [example_run_source, verbosities + verbosities_run] }
}
