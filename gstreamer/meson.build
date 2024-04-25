sub_dir_name = meson.current_source_dir().split('/').get(-1)

text_plugin_files = [files([
                               'factories/TEXT/gstreamerTEXTFactory.cc',
                               'factories/TEXT/gstreamerTEXTConnection.cc',
                               'factories/TEXT/event/event.cc',
                               'factories/TEXT/event/eventHeader.cc',
                               'factories/TEXT/event/publishTrueInfo.cc',
                               'factories/TEXT/event/publishDigitized.cc',
                               'factories/TEXT/stream/stream.cc',
                               'factories/TEXT/stream/frameHeader.cc',
                               'factories/TEXT/stream/publishPayload.cc'
                           ]),
                     true]

jlab_sro_plugin_files = [files([
                                   'factories/JLABSRO/gstreamerJLABSROFactory.cc',
                                   'factories/JLABSRO/gstreamerJLABSROConnection.cc',
                                   'factories/JLABSRO/stream/stream.cc',
                                   'factories/JLABSRO/stream/frameHeader.cc',
                                   'factories/JLABSRO/stream/publishPayload.cc'
                               ]),
                         true]

root_plugin_files = [files([
                               'factories/ROOT/gRootTree.cc',
                               'factories/ROOT/gstreamerROOTFactory.cc',
                               'factories/ROOT/gstreamerROOTConnection.cc',
                               'factories/ROOT/event/event.cc',
                               'factories/ROOT/event/eventHeader.cc',
                               'factories/ROOT/event/publishTrueInfo.cc',
                               'factories/ROOT/event/publishDigitized.cc',
                               'factories/ROOT/stream/stream.cc',
                               'factories/ROOT/stream/frameHeader.cc',
                               'factories/ROOT/stream/publishPayload.cc'
                           ]),
                     true]


if root_deps.found()
    streamer_plugins = {
        'gstreamerTEXTFactory' : text_plugin_files,
        'gstreamerJLABSROFactory' : jlab_sro_plugin_files,
        'gstreamerROOTFactory' : root_plugin_files
    }
    streamer_dependencies = [geant4_deps, clhep_deps, root_deps]
else
    streamer_plugins = {
        'gstreamerTEXTFactory' : text_plugin_files,
        'gstreamerJLABSROFactory' : jlab_sro_plugin_files
    }
    streamer_dependencies = [geant4_deps, clhep_deps]
endif

LD += {
    'name' : sub_dir_name,
    'sources' : files([
                          'gstreamer.cc',
                          'gstreamerOptions.cc'
                      ]),
    'headers' : files([
                          'gstreamer.h',
                          'gstreamerConventions.h',
                          'gstreamerOptions.h'
                      ]),
    'plugins' : streamer_plugins,
    'dependencies' : streamer_dependencies,
    'examples' : {
        'gstreamer_example' : [files(['examples/example.cc']), ['']],
    }
}
