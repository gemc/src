sub_dir_name = meson.current_source_dir().split('/').get(-1)
LD += {
    'name' : sub_dir_name,
    'sources' : files(
        'textProgressBar.cc'
    ),
    'headers' : files(
        'textProgressBar.h',
        'textProgressBarConventions.h'
    ),
    'examples' : {
        'test_textProgressBar' : [files('examples/bar.cc'), '']
    }
}
