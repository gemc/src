sub_dir_name = meson.current_source_dir().split('/').get(-1)
internal_deps = ['goptions', 'guts', 'glogging']

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

    # expat/zlib are geant4's static-link needs (no source here compiles against them);
    # they are linked once at the final executables to avoid duplicate -lexpat/-lz.
    'dependencies' : [yaml_cpp_dep, clhep_deps, geant4_core_deps],
    'internal_dependencies' : internal_deps,

    'examples' : {
        'test_gphysics_verbose' : [example_source, verbosities]
    }
}
