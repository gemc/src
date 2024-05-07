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
        'gmagneto.cc'
    ),
    'headers' : files(
        'gfield.h',
        'gfieldOptions.h',
        'gfieldConventions.h',
        'gmagneto.h'
    ),
    'plugins' : {
        'gfield_multipoles' : multipoles_files,
    },

    'dependencies' : [clhep_deps, geant4_deps],
    'examples' : {
        'dipole' : [files('examples/dipole.cc'), ['']],
    }
}
