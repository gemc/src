sub_dir_name = meson.current_source_dir().split('/').get(-1)
internal_deps = []

with_gui = ['']
if get_option('i_test')
    with_gui = ['-gui']
endif

example_source = files('examples/g4dialog_example.cc')
verbosities = [
    '-verbosity.g4dialog=2',
    '-debug.g4dialog=true'
]

sources = files(
    'g4dialog.cc',
    'g4dialog_options.cc',
    'tabs/gcommands.cc',
)

headers = files(
    'g4dialog.h',
    'g4dialog_options.h',
)

moc_headers = files(
    'tabs/gcommands.h',
)

examples = {
    'test_g4dialog' : [example_source, with_gui],
    'test_g4dialog_verbose' : [example_source, with_gui + verbosities]
}


LD += {
    'name' : sub_dir_name,
    'sources' : sources,
    'headers' : headers,
    'moc_headers' : moc_headers,
    'dependencies' : [yaml_cpp_dep, qt6_deps, clhep_deps, geant4_deps],
    'internal_dependencies' : internal_deps,
    'additional_includes' : ['g4dialog', 'g4dialog/tabs'],
    'examples' : examples
}
