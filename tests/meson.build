# pygemc geometry + gemc binary tests.
# CLI and Python-only tests are covered by pygemc's own pytest CI.
# Here we only run what requires the gemc binary, with the geometry-building
# steps as explicit prerequisites (higher priority = runs first).

system_template = find_program('gemc-system-template', dirs: [venv_bin])

build_root = meson.project_build_root()

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
