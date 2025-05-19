# expat needed because g4processes
sub_dir_name = meson.current_source_dir().split('/').get(-1)
LD += {
    'name' : sub_dir_name,
    'sources' : files(
                          'gphysics.cc',
                          'gphysics_options.cc'
                      ),
    'headers' : files(
                          'gphysics.h',
                          'gphysics_options.h',
                          'gphysicsConventions.h'
                      ),
    'dependencies' : [yaml_cpp_dep, clhep_deps, geant4_deps, expat_dep],
    'examples' : {
        'test_gphysics' : [files('examples/gphysics_example.cc'), '' ],
        'test_gphysics_verbose' : [files('examples/gphysics_example.cc'), '-verbosity.gphysics=2', '-debug.gphysics=true']
    }
}
