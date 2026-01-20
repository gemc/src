sub_dir_name = meson.current_source_dir().split('/').get(-1)
internal_deps = []

example_sourcess = files('examples/bar.cc')

LD += {
    'name' : sub_dir_name,
    'sources' : files(
        'textProgressBar.cc'
    ),
    'headers' : files(
        'textProgressBar.h',
        'textProgressBarConventions.h'
    ),

    'dependencies' : [],
    'internal_dependencies' : internal_deps,

    'examples' : {
        'test_textProgressBar' : [example_sourcess, '']
    }
}
