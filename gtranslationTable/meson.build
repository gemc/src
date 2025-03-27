sub_dir_name = meson.current_source_dir().split('/').get(-1)
LD += {
    'name' : sub_dir_name,
    'sources' : files(
        'gtranslationTable.cc',
        'gtranslationTable_options.cc'
    ),
    'headers' : files(
        'gtranslationTable.h',
        'gtranslationTable_options.h',
        'gtranslationTableConventions.h'
    ),
    'dependencies' : [yaml_cpp_dep, clhep_deps, geant4_deps],
    'examples' : {
        'test_translation_table' : [files('examples/tt_example.cc'), ''],
        'test_translation_table_debug' : [files('examples/tt_example.cc'), '-debug.gtranslationTable=true'],
        'test_translation_table_verbosity' : [files('examples/tt_example.cc'), '-verbosity.gtranslationTable=2']
    }
}
