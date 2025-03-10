sub_dir_name = meson.current_source_dir().split('/').get(-1)

ascii_plugin_files = [files(
                         'factories/ASCII/gstreamerASCIIFactory.cc',
                         'factories/ASCII/gstreamerASCIIConnection.cc',
                         'factories/ASCII/event/event.cc',
                         'factories/ASCII/event/eventHeader.cc',
                         'factories/ASCII/event/publishTrueInfo.cc',
                         'factories/ASCII/event/publishDigitized.cc',
                         'factories/ASCII/stream/stream.cc',
                         'factories/ASCII/stream/frameHeader.cc',
                         'factories/ASCII/stream/publishPayload.cc'
                     ),
                     true]

jlab_sro_plugin_files = [files(
                             'factories/JLABSRO/gstreamerJLABSROFactory.cc',
                             'factories/JLABSRO/gstreamerJLABSROConnection.cc',
                             'factories/JLABSRO/stream/stream.cc',
                             'factories/JLABSRO/stream/frameHeader.cc',
                             'factories/JLABSRO/stream/publishPayload.cc'
                         ),
                         true]

root_plugin_files = [files(
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
                     ),
                     true]


if get_option('use_root')
    if root_deps.found()
        streamer_plugins = {
            'gstreamerASCIIFactory' : ascii_plugin_files,
            'gstreamerJLABSROFactory' : jlab_sro_plugin_files,
            'gstreamerROOTFactory' : root_plugin_files
        }
        streamer_dependencies = [yaml_cpp_dep, clhep_deps, geant4_deps, root_deps]

    endif
else
    streamer_plugins = {
        'gstreamerASCIIFactory' : ascii_plugin_files,
        'gstreamerJLABSROFactory' : jlab_sro_plugin_files
    }
    streamer_dependencies = [yaml_cpp_dep, clhep_deps, geant4_deps]
endif

LD += {
    'name' : sub_dir_name,
    'sources' : files(
        'gstreamer.cc',
        'gstreamerOptions.cc'
    ),
    'headers' : files(
        'gstreamer.h',
        'gstreamerConventions.h',
        'gstreamerOptions.h'
    ),
    'plugins' : streamer_plugins,
    'dependencies' : streamer_dependencies,
    'examples' : {
        'test_gstreamer' : [files('examples/example.cc'), ['']],
    }
}
