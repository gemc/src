sub_dir_name = meson.current_source_dir().split('/').get(-1)

example_source = files('examples/gphysics_example.cc')

verbosities = ['-verbosity.gphysics=2',
               '-debug.gphysics=true',
]

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
    'dependencies' : [yaml_cpp_dep, clhep_deps, geant4_deps, expat_dep], # expat needed because g4processes
    'examples' : {
        'test_gphysics' : [example_source, '' ],
        'test_gphysics_verbose' : [example_source, verbosities]
    }
}
