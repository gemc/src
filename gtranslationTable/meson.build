sub_dir_name = meson.current_source_dir().split('/').get(-1)
LD += {
    'name' : sub_dir_name,
    'sources' : files([
                          'gtranslationTable.cc'

                      ]),
    'headers' : files([
                          'gtranslationTable.h',
                          'gtranslationTableConventions.h'
                      ]),
    'examples' : {
        'basic_tt' : [files(['examples/simple.cc']), ['']],
    }
}
