sub_dir_name = meson.current_source_dir().split('/').get(-1)
verbosities = ['-verbosity.plugins=2',
               '-verbosity.gdigitization=2',
               '-verbosity.gtouchable=2',
               '-verbosity.gstreamer=2',
               '-debug.plugins=true',
               '-debug.gtouchable=true',
               '-debug.gstreamer=true',
]
root = ['-gstreamer="[{format: root, filename: out}]"']
ascii = ['-gstreamer="[{format: ascii, filename: out}]"']
ascii_root = ['-gstreamer="[{format: ascii, filename: out}, {format: root, filename: out}]"']
buffer = ['-ebuffer=20']

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

# TODO: just detect if root is available
if get_option('use_root')
    if root_deps.found()
        streamer_plugins = {
            'gstreamer_ascii_plugin' : ascii_plugin_files,
            'gstreamer_jlabsro_plugin' : jlab_sro_plugin_files,
            'gstreamer_root_plugin' : root_plugin_files
        }
        streamer_dependencies = [yaml_cpp_dep, clhep_deps, geant4_deps, root_deps]

    endif
else
    streamer_plugins = {
        'gstreamer_ascii_plugin' : ascii_plugin_files,
        'gstreamer_jlabsro_plugin' : jlab_sro_plugin_files
    }
    streamer_dependencies = [yaml_cpp_dep, clhep_deps, geant4_deps]
endif

LD += {
    'name' : sub_dir_name,
    'sources' : files(
        'gstreamer.cc',
        'gstreamer_options.cc'
    ),
    'headers' : files(
        'gstreamer.h',
        'gstreamerConventions.h',
        'gstreamer_options.h'
    ),
    'plugins' : streamer_plugins,
    'dependencies' : streamer_dependencies,
    'additional_includes' : ['gstreamer', 'gstreamer/factories/ROOT', 'gstreamer/factories/ASCII', 'gstreamer/factories/JLABSRO'],
    'examples' : {
        'test_gstreamer' : [files('examples/gstreamer_example.cc'), buffer],
        'test_gstreamer_verbose' : [files('examples/gstreamer_example.cc'), buffer + verbosities],
        'test_gstreamer_root' : [files('examples/gstreamer_example.cc'), buffer + root],
        'test_gstreamer_root_verbose' : [files('examples/gstreamer_example.cc'),  buffer + root + verbosities],
        'test_gstreamer_ascii' : [files('examples/gstreamer_example.cc'),  buffer + ascii],
        'test_gstreamer_ascii_verbose' : [files('examples/gstreamer_example.cc'),  buffer + ascii + verbosities],
        'test_gstreamer_ascii_root' : [files('examples/gstreamer_example.cc'),  buffer + ascii_root],
        'test_gstreamer_ascii_root_verbose' : [files('examples/gstreamer_example.cc'),  buffer + ascii_root + verbosities]
    }
}
