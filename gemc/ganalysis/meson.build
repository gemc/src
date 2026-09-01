sub_dir_name = meson.current_source_dir().split('/').get(-1)
internal_deps = ['goptions', 'gdata']

# The data engine (accumulator, options, histogram) has no Qt Charts dependency and always builds.
# The on-screen view (gAnalysisView) and the examples that instantiate it link Qt6Charts, so they are
# added only when Charts was found at configure time (qt6_charts_found, set in meson/meson.build).
ganalysis_sources = [
    'gAnalysisAccumulator.cc',
    'gAnalysisOptions.cc',
    'gHistogram.cc',
]
ganalysis_headers = [
    'gAnalysisAccumulator.h',
    'gAnalysisOptions.h',
    'gHistogram.h',
]
ganalysis_deps = [yaml_cpp_dep, clhep_deps, geant4_core_deps, qt6_deps]
ganalysis_example_internal_deps = {
    'test_ganalysis_accumulator' : ['ghit', 'gtouchable'],
}
ganalysis_examples = {
    'test_ganalysis_accumulator' : [files('examples/analysis_accumulator_example.cc'), ''],
}

if qt6_charts_found
    ganalysis_sources += 'gAnalysisView.cc'
    ganalysis_headers += 'gAnalysisView.h'
    ganalysis_deps += qt6_charts_dep
    ganalysis_example_internal_deps += {
        'test_ganalysis_histogram' : ['ghit', 'gtouchable'],
        'test_ganalysis_runtime_integration' : ['ghit', 'gtouchable'],
    }
    ganalysis_examples += {
        'test_ganalysis_histogram' : [files('examples/analysis_histogram_example.cc'), ''],
        'test_ganalysis_runtime_integration' : [
            files('examples/analysis_runtime_integration_example.cc'), ''
        ],
    }
endif

LD += {
    'name' : sub_dir_name,
    'sources' : files(ganalysis_sources),
    'headers' : files(ganalysis_headers),
    'additional_includes' : ['gemc/ganalysis'],
    'dependencies' : ganalysis_deps,
    'internal_dependencies' : internal_deps,
    'example_internal_dependencies' : ganalysis_example_internal_deps,
    'examples' : ganalysis_examples
}
