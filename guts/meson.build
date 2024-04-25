sub_dir_name = meson.current_source_dir().split('/').get(-1)
LD += {
    'name' : sub_dir_name,
    'sources' : files([
                          'gutilities.cc',
                          'gutsConventions.cc'
                      ]),
    'headers' : files([
                          'gutilities.h',
                          'gutsConventions.h'
                      ]),

    'dependencies' : [clhep_deps],
    'examples' : {
        'guts_string_vector_from_string' : [files(['examples/string_vector_from_string.cc']), ['akasha enkil marius']]
    }
}
