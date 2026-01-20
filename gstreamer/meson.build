# ── helpers / common ───────────────────────────────────────────────────────────
sub_dir_name = meson.current_source_dir().split('/').get(-1)  # fine on *nix
internal_deps = ['gtouchable', 'gdata', 'gtranslationTable', 'ghit']

example_source = files('examples/gstreamer_example.cc')
verbosities = [
    '-verbosity.plugins=2',
    '-verbosity.gdigitization=2',
    '-verbosity.gtouchable=2',
    '-verbosity.gstreamer=2',
    '-debug.plugins=true',
    '-debug.gtouchable=true',
    '-debug.gstreamer=true',
]

# output format args
fmt_args = {
    'root' : ['-gstreamer="[{format: root,  filename: out}]"'],
    'ascii' : ['-gstreamer="[{format: ascii, filename: out}]"'],
    'csv' : ['-gstreamer="[{format: csv,   filename: out}]"'],
    'ascii_root' : ['-gstreamer="[{format: ascii, filename: out}, {format: root, filename: out}]"'],
}

buffer = ['-ebuffer=20']

# plugin file lists (unchanged)
ascii_plugin_files = [files(
                          'factories/ASCII/gstreamerASCIIFactory.cc',
                          'factories/ASCII/gstreamerASCIIConnection.cc',
                          'factories/ASCII/event/event.cc',
                          'factories/ASCII/event/eventHeader.cc',
                          'factories/ASCII/event/publishTrueInfo.cc',
                          'factories/ASCII/event/publishDigitized.cc',
                          'factories/ASCII/stream/stream.cc',
                          'factories/ASCII/stream/frameHeader.cc',
                          'factories/ASCII/stream/publishPayload.cc',
                      ), true]

jlab_sro_plugin_files = [files(
                             'factories/JLABSRO/gstreamerJLABSROFactory.cc',
                             'factories/JLABSRO/gstreamerJLABSROConnection.cc',
                             'factories/JLABSRO/stream/stream.cc',
                             'factories/JLABSRO/stream/frameHeader.cc',
                             'factories/JLABSRO/stream/publishPayload.cc',
                         ), true]

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
                         'factories/ROOT/stream/publishPayload.cc',
                     ), true]

csv_plugin_files = [files(
                        'factories/CSV/gstreamerCSVFactory.cc',
                        'factories/CSV/gstreamerCSVConnection.cc',
                        'factories/CSV/event/event.cc',
                        'factories/CSV/event/eventHeader.cc',
                        'factories/CSV/event/publishTrueInfo.cc',
                        'factories/CSV/event/publishDigitized.cc',
                        'factories/CSV/stream/stream.cc',
                        'factories/CSV/stream/frameHeader.cc',
                        'factories/CSV/stream/publishPayload.cc',
                    ), true]

# ── assemble plugins/deps/includes once, then extend if ROOT is present ───────
streamer_plugins = {
    'gstreamer_ascii_plugin' : ascii_plugin_files,
    'gstreamer_csv_plugin' : csv_plugin_files,
    'gstreamer_jlabsro_plugin' : jlab_sro_plugin_files,
}

streamer_dependencies = [yaml_cpp_dep, clhep_deps, geant4_core_deps]
additional_includes = ['gstreamer', 'gstreamer/factories/ASCII', 'gstreamer/factories/CSV', 'gstreamer/factories/JLABSRO']

# Add ROOT bits if available
if root_dep.found()
    streamer_plugins += { 'gstreamer_root_plugin' : root_plugin_files }
    streamer_dependencies += root_dep
    additional_includes += ['gstreamer/factories/ROOT']
endif

# ── examples: generate the matrix programmatically ────────────────────────────
examples = {
    'test_gstreamer' : [example_source, buffer],
    'test_gstreamer_verbose' : [example_source, buffer + verbosities],
}

# Always-available formats
foreach name : ['ascii', 'csv']
    fmt = fmt_args.get(name)
    examples += {
        'test_gstreamer_' + name : [example_source, buffer + fmt],
        'test_gstreamer_' + name + '_verbose' : [example_source, buffer + fmt + verbosities],
    }
endforeach

# ROOT-only examples (guarded)
if root_dep.found()
    foreach name : ['root', 'ascii_root']
        fmt = fmt_args.get(name)
        examples += {
            'test_gstreamer_' + name : [example_source, buffer + fmt],
            'test_gstreamer_' + name + '_verbose' : [example_source, buffer + fmt + verbosities],
        }
    endforeach
endif

# ── single LD append with one dict literal ────────────────────────────────────
LD += {
    'name' : sub_dir_name,
    'sources' : files('gstreamer.cc', 'gstreamer_options.cc'),
    'headers' : files('gstreamer.h', 'gstreamerConventions.h', 'gstreamer_options.h'),
    'plugins' : streamer_plugins,
    'dependencies' : streamer_dependencies,
    'internal_dependencies' : internal_deps,
    'additional_includes' : additional_includes,
    'examples' : examples,
}
