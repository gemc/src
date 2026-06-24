sub_dir_name = meson.current_source_dir().split('/').get(-1)
internal_deps = ['goptions', 'guts', 'glogging', 'gfactory']

example_source = files('examples/test_gfield_dipole.cc')
verbosities = ['-verbosity.gfield=2',
               '-debug.gfield=true'
]
input_file = [meson.current_source_dir() + '/examples/dipole.yaml']
field_points_file = meson.current_source_dir() + '/examples/field_points.txt'


multipoles_files = [files(
                        'gfieldFactories/multipoles/gfield_multipoles.cc',
                    ),
                    true]

asciimap_files = [files(
                      'gfieldFactories/asciimap/gfield_asciimap.cc',
                  ),
                  true]

# Example ASCII maps. The YAML definitions are plain, directly-runnable files; the asciimap plugin
# resolves each data-only map next to its YAML, so passing the YAML by absolute path (as the tests do)
# finds the map without any configure/install step.
asciimap_dipole_yaml   = meson.current_source_dir() + '/examples/asciimap_dipole.yaml'
asciimap_solenoid_yaml = meson.current_source_dir() + '/examples/solenoid.yaml'
asciimap_torus_yaml    = meson.current_source_dir() + '/examples/torus.yaml'

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
        'gfieldasciimapFactory' : asciimap_files,
    },
    'additional_includes' : ['gemc/gfields',
                             'gemc/gfields/gfieldFactories/multipoles',
                             'gemc/gfields/gfieldFactories/asciimap'],

    'dependencies' : [yaml_cpp_dep, clhep_deps, geant4_core_deps],
    'internal_dependencies' : internal_deps,

    'examples' : {
        'test_gfield_dipole' : [example_source, input_file],
        'test_gfield_dipole_verbose' : [example_source, input_file + verbosities],
        'test_gfield_dipole_field_at' : [example_source, input_file + ['-fieldAt=10*cm 0*cm 0*cm']],
        'test_gfield_dipole_field_map_points' : [
            example_source,
            input_file + ['-fieldMapPoints=' + field_points_file],
        ],
        'test_gfield_asciimap_dipole' : [example_source, [asciimap_dipole_yaml]],
        'test_gfield_asciimap_dipole_field_at' : [
            example_source,
            [asciimap_dipole_yaml, '-fieldAt=0*cm 1*cm 0*cm'],
        ],
        'test_gfield_asciimap_solenoid' : [example_source, [asciimap_solenoid_yaml]],
        'test_gfield_asciimap_solenoid_field_at' : [
            example_source,
            [asciimap_solenoid_yaml, '-fieldAt=1*m 0*m 0*m'],
        ],
        'test_gfield_asciimap_torus' : [example_source, [asciimap_torus_yaml]],
        'test_gfield_asciimap_torus_field_at' : [
            example_source,
            [asciimap_torus_yaml, '-fieldAt=250*cm 0*cm 350*cm'],
        ],
    }
}
