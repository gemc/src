sub_dir_name = meson.current_source_dir().split('/').get(-1)
LD += {
    'name' : sub_dir_name,
    'sources' : files(
        'gutilities.cc'
    ),
    'headers' : files(
        'gutilities.h',
        'gutsConventions.h'
    ),

    'dependencies' : [yaml_cpp_dep, clhep_deps, geant4_deps],
    'examples' : {
        'test_guts_string_vector_from_string' : [files('examples/string_vector_from_string.cc'), 'akasha enkil marius']
    }
}
