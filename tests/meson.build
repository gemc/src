# pygemc geometry + gemc binary tests.
# CLI and Python-only tests are covered by pygemc's own pytest CI.
# Here we only run what requires the gemc binary, with the geometry-building
# steps as explicit prerequisites (higher priority = runs first).

system_template = find_program('gemc-system-template', dirs: [venv_bin])

build_root = meson.project_build_root()

gfield_random_state = executable(
    'test_gfield_random_state',
    'gfield_random_state.cc',
    include_directories: gemc_includes,
    dependencies: [yaml_cpp_dep, clhep_deps, geant4_core_deps, expat_dep, zlib_dep,
                   dependency('threads')],
    link_with: [lib_target_maps['gfields'], lib_target_maps['gfactory'],
                lib_target_maps['glogging'], lib_target_maps['goptions'], lib_target_maps['guts']],
    export_dynamic: true)
test('gfield_random_state', gfield_random_state,
     args: [meson.project_source_root() / 'gemc/gfields/examples/dipole.yaml',
            '-plugin_path=' + build_root],
     depends: [plugin_target_maps['gfieldmultipolesFactory']],
     env: project_test_env, suite: 'gfields')

test('gui_catalogs', python_exe,
     args: [files('qt_catalogs.py')], env: project_test_env, suite: 'gui')
test('gui_catalog_freshness', python_exe,
     args: [files('../ci/qt_catalogs.py'), 'check'], env: project_test_env, suite: 'gui')

gui_probe_dependencies = [yaml_cpp_dep, qt6_deps, clhep_deps, geant4_deps, ogl_deps,
                          assimp_dep, expat_dep, zlib_dep]
gui_probe_arguments = []
if qt6_svg_dep.found()
    gui_probe_dependencies += qt6_svg_dep
    gui_probe_arguments += '-DGEMC_TEST_QTSVG'
endif
gui_translation = executable(
    'test_gui_translation',
    files('gui_translation.cc', '../gemc_options.cc') + qrc_gemc_sources,
    include_directories: gemc_includes,
    dependencies: gui_probe_dependencies,
    cpp_args: gui_probe_arguments,
    link_with: all_libs,
    export_dynamic: true)
test('gui_translation', python_exe,
     args: [files('gui_translation.py'), gui_translation, meson.project_source_root(), qt_gui_lrelease],
     env: project_test_env, suite: 'gui', timeout: 300)

glogger_templates = executable(
    'test_glogger_templates',
    'glogger_templates.cc',
    include_directories: gemc_includes,
    dependencies: [yaml_cpp_dep, clhep_deps, geant4_core_deps, expat_dep, zlib_dep],
    link_with: [lib_target_maps['glogging'], lib_target_maps['goptions'], lib_target_maps['guts']])

test('glogging_templates',
     python_exe,
     args: [files('glogger_templates.py'), glogger_templates],
     env: project_test_env,
     suite: 'glogging',
     timeout: 180)

test('glogging_catalogs',
     python_exe,
     args: [files('message_catalogs.py')],
     env: project_test_env,
     suite: 'glogging')

test('glogging_catalog_freshness',
     python_exe,
     args: [files('../ci/extract_messages.py'), 'extract', '--check'],
     env: project_test_env,
     suite: 'glogging')

test('glogging_project_catalogs',
     python_exe,
     args: [files('../ci/extract_messages.py'), 'validate-tree'],
     env: project_test_env,
     suite: 'glogging')

test('glogging_project_languages',
     python_exe,
     args: [files('glogger_catalog_runtime.py'), gemc, meson.project_source_root() / 'translations'],
     env: project_test_env,
     suite: 'glogging',
     timeout: 180)

glogger_translation = executable(
    'test_glogger_translation',
    'glogger_translation.cc',
    include_directories: gemc_includes,
    dependencies: [yaml_cpp_dep, clhep_deps, geant4_core_deps, expat_dep, zlib_dep],
    link_with: [lib_target_maps['glogging'], lib_target_maps['goptions'], lib_target_maps['guts']],
    export_dynamic: true)

glogger_translation_plugin = shared_module(
    'glogger_translation_probe',
    'glogger_translation_plugin.cc',
    include_directories: gemc_includes,
    dependencies: declare_dependency(dependencies: [yaml_cpp_dep, clhep_deps, geant4_core_deps])
                  .partial_dependency(compile_args: true, includes: true),
    name_prefix: '',
    name_suffix: 'gplugin',
    override_options: ['b_lundef=false'])

test('glogging_translation',
     python_exe,
     args: [files('glogger_translation.py'), glogger_translation, glogger_translation_plugin],
     env: project_test_env,
     suite: 'glogging',
     timeout: 180)

test('geant4_macro',
     python_exe,
     args: [files('geant4_macro.py'), gemc],
     env: project_test_env,
     suite: 'macro',
     timeout: 180)

g4objects = {
    'G4Box'    : '10 10 10 cm',
    'G4Tubs'   : '0, 20, 40, 0, 360 cm deg',
    'G4Cons'   : '10 20 30 40 10 0 360 cm deg',
    'G4Trd'    : '30 10 40 15 60 cm',
    'G4TrapRAW': '10 20 30 40',
    'G4TrapG'  : '60 20 5 40 30 40 10 16 10 14 10 cm deg'
}

# --- basic 'test' system: create → build → run gemc ---

test_dir = join_paths(build_root, 'test')
run_command('mkdir', '-p', test_dir, check: true)

test('api_create_system',
     system_template,
     is_parallel: false,
     env: test_env,
     args: ['-s', 'test'],
     workdir: build_root,
     priority: 5)

foreach format : ['ascii', 'sqlite']
    test('api_build_geometry_for_format_' + format,
         python_exe,
         is_parallel: false,
         env: test_env,
         args: ['test.py', '-f', format],
         workdir: test_dir,
         priority: -1)
    test('api_run_gemc_for_format_' + format,
         gemc,
         args: ['test.yaml', '-gsystem="[{name: test, factory: ' + format + '}]"'],
         workdir: test_dir,
         is_parallel: false,
         env: project_test_env,
         priority: -2)
endforeach

# --- per-solid: create → build → run gemc ---

foreach volume : g4objects.keys()
    test_name      = 'test_' + volume
    yaml_file      = test_name + '.yaml'
    python_main    = test_name + '.py'
    solid_test_dir = join_paths(build_root, test_name)
    run_command('mkdir', '-p', solid_test_dir, check: true)

    test('api_create_template_system_with_' + volume,
         system_template,
         is_parallel: false,
         env: test_env,
         args: ['-s', test_name],
         workdir: build_root,
         priority: 4)

    foreach format : ['ascii', 'sqlite']
        test('api_build_geometry_with_' + volume + '_format_' + format,
             python_exe,
             is_parallel: false,
             env: test_env,
             args: [python_main, '-f', format],
             workdir: solid_test_dir,
             priority: -4)
        test('api_run_gemc_with_' + volume + '_format_' + format,
             gemc,
             args: [yaml_file, '-gsystem="[{name: ' + test_name + ', factory: ' + format + '}]"'],
             workdir: solid_test_dir,
             is_parallel: false,
             env: project_test_env,
             priority: -30)
    endforeach
endforeach
