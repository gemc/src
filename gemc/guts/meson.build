sub_dir_name = meson.current_source_dir().split('/').get(-1)
internal_deps = []

example_source = files('examples/string_vector_from_string.cc')
options = ['akasha enkil marius']

LD += {
    'name' : sub_dir_name,
    'sources' : files(
        'gutilities.cc'
    ),
    'headers' : files(
        'gthreads.h',
        'gutilities.h',
        'gutsConventions.h'
    ),

    'dependencies' : [yaml_cpp_dep, clhep_deps, geant4_core_deps],
    'internal_dependencies' : internal_deps,

    'examples' : {
        'test_guts_string_vector_from_string' : [example_source, options]
    }
}
