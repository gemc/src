sub_dir_name = meson.current_source_dir().split('/').get(-1)
internal_deps = ['goptions', 'gdata']

LD += {
    'name' : sub_dir_name,
    'sources' : files(
        'gAnalysisAccumulator.cc',
        'gAnalysisOptions.cc',
        'gAnalysisView.cc',
        'gHistogram.cc'
    ),
    'headers' : files(
        'gAnalysisAccumulator.h',
        'gAnalysisOptions.h',
        'gAnalysisView.h',
        'gHistogram.h'
    ),
    'additional_includes' : ['gemc/ganalysis'],
    'dependencies' : [yaml_cpp_dep, clhep_deps, geant4_core_deps, qt6_deps, qt6_charts_dep],
    'internal_dependencies' : internal_deps,
    'examples' : {
        'test_ganalysis_accumulator' : [files('examples/analysis_accumulator_example.cc'), ''],
        'test_ganalysis_histogram' : [files('examples/analysis_histogram_example.cc'), ''],
        'test_ganalysis_runtime_integration' : [
            files('examples/analysis_runtime_integration_example.cc'), ''
        ]
    }
}
