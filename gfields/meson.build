sub_dir_name = meson.current_source_dir().split('/').get(-1)

multipoles_files = [files(
                        'gfieldFactories/multipoles/gfield_multipoles.cc',
                    ),
                    true]

LD += {
    'name' : sub_dir_name,
    'sources' : files(
        'gfield.cc',
        'gfieldOptions.cc',
        'gmagneto.cc',
    ),
    'headers' : files(
        'gfield.h',
        'gfieldOptions.h',
        'gfieldConventions.h',
        'gmagneto.h',

    ),
    'plugins' : {
        'gfieldmultipolesFactory' : multipoles_files,
    },
    'additional_includes' : ['gfields', 'gfields/gfieldFactories/multipoles'],
    'dependencies' : [yaml_cpp_dep, clhep_deps, geant4_deps],
    'examples' : {
        'test_dipole' : [files('examples/example.cc'), [meson.current_source_dir() + '/examples/dipole.yaml']],
    }
}
