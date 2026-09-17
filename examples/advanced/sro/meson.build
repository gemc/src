simple_sro_includes = []
foreach directory : all_includes
    simple_sro_includes += include_directories('../../..' / directory)
endforeach
simple_sro_dependencies = [yaml_cpp_dep, clhep_deps, geant4_core_deps]
if not use_sharedl
    # Static GEMC libraries already propagate their link dependencies.
    simple_sro_dependencies = declare_dependency(dependencies : simple_sro_dependencies).partial_dependency(
        compile_args : true, includes : true)
endif

simple_sro_plugin = shared_library(
    'example_sro', 'sro_plugin.cc',
    name_prefix : '', name_suffix : 'gplugin', install : true,
    include_directories : simple_sro_includes,
    link_with : [lib_target_maps['gstreamer'], lib_target_maps['gdynamicDigitization']],
    dependencies : [simple_sro_dependencies, zlib_dep],
)

# Generate geometry and run GEMC in a temporary directory, independent of installed example artifacts.
test('examples_sro_adc', python_exe,
     args : [files('check_example.py'), gemc.full_path(), simple_sro_plugin.full_path()],
     depends : [gemc, simple_sro_plugin],
     env : project_test_env,
     suite : ['sro', 'threading'],
     timeout : 60)
