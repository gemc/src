sub_dir_name = meson.current_source_dir().split('/').get(-1)
internal_deps = ['goptions', 'guts', 'gtouchable']

example_source = files('examples/tt_example.cc')
verbosities = ['-verbosity.gtranslationtable=2',
               '-debug.gtranslationtable=true'
]


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

    'dependencies' : [yaml_cpp_dep, clhep_deps, geant4_core_deps],
    'internal_dependencies' : internal_deps,

    'examples' : {
        'test_translation_table' : [example_source, ''],
        'test_translation_table_verbose' : [example_source, verbosities]
    }
}
