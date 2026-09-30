simple_sro_includes = []
foreach directory : all_includes
    simple_sro_includes += include_directories('../../..' / directory)
endforeach
simple_sro_dependencies = [yaml_cpp_dep, clhep_deps, geant4_core_deps]
simple_sro_link_with = [lib_target_maps['gstreamer'], lib_target_maps['gdynamicDigitization'],
                        lib_target_maps['gparticle']]
if not use_sharedl
    # GEMC exports the framework symbols used by this runtime-loaded module.
    # Embedding static framework/Geant4 archives adds initial-exec TLS storage that can
    # exhaust Linux's static TLS reserve during dlopen, especially on ARM64.
    simple_sro_link_with = []
    simple_sro_dependencies = declare_dependency(dependencies : simple_sro_dependencies).partial_dependency(
        compile_args : true, includes : true)
endif

simple_sro_plugin = shared_module(
    'example_sro', 'sro_plugin.cc',
    name_prefix : '', name_suffix : 'gplugin', install : true,
    include_directories : simple_sro_includes,
    link_with : simple_sro_link_with,
    dependencies : [simple_sro_dependencies, zlib_dep],
)

# Generate geometry and run GEMC in a temporary directory, independent of installed example artifacts.
test('examples_sro_adc', python_exe,
     args : [files('check_example.py'), gemc.full_path(), simple_sro_plugin.full_path()],
     depends : [gemc, simple_sro_plugin],
     env : project_test_env,
     suite : ['sro', 'threading'],
     timeout : 60)
