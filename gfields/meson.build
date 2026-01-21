sub_dir_name = meson.current_source_dir().split('/').get(-1)
internal_deps = ['goptions', 'guts', 'glogging']

example_source = files('examples/test_gfield_dipole.cc')
verbosities = ['-verbosity.gfield=2',
               '-debug.gfield=true'
]
input_file = [meson.current_source_dir() + '/examples/dipole.yaml']


multipoles_files = [files(
                        'gfieldFactories/multipoles/gfield_multipoles.cc',
                    ),
                    true]

LD += {
    'name' : sub_dir_name,
    'sources' : files(
        'gfield.cc',
        'gfield_options.cc',
        'gmagneto.cc',
    ),
    'headers' : files(
        'gfield.h',
        'gfield_options.h',
        'gfieldConventions.h',
        'gmagneto.h',

    ),
    'plugins' : {
        'gfieldmultipolesFactory' : multipoles_files,
    },
    'additional_includes' : ['gfields', 'gfields/gfieldFactories/multipoles'],

    'dependencies' : [yaml_cpp_dep, clhep_deps, geant4_core_deps],
    'internal_dependencies' : internal_deps,

    'examples' : {
        'test_gfield_dipole' : [example_source, input_file],
        'test_gfield_dipole_verbose' : [example_source, input_file + verbosities],
    }
}
