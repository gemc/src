api_dir = join_paths(get_option('prefix'), 'api')
build_root = meson.project_build_root()

test_env = environment()
test_env.set('PYTHONDONTWRITEBYTECODE', '1')
test_env.prepend('PYTHONPATH', api_dir)
python_exe = import('python').find_installation('python3')
system_template = join_paths(api_dir, 'system_template.py')

install_subdir(meson.current_source_dir(), install_dir : '.')

test('api_template_help',
     python_exe,
     env: test_env,
     args : [system_template, '-h'])

test('api_show_solid_creators',
     python_exe,
     env: test_env,
     args : [system_template, '-sl'],
)

# Test all solid creators
g4objects = {
    'G4Box' : '10 10 10 cm',
    'G4Tubs' : '0, 20, 40, 0, 360 cm deg',
    'G4Cons' : '10 20 30 40 10 0 360 cm deg',
    'G4Trd' : '30 10 40 15 60 cm',
    'G4TrapRAW' : '10 20 30 40',
    'G4TrapG' : '60 20 5 40 30 40 10 16 10 14 10 cm deg'
}


foreach volume: g4objects.keys()
    pars = g4objects[volume]
    test('api_show_template_code_for_' + volume,
         python_exe,
         is_parallel: false,
         env: test_env,
         args : [system_template, '-gv', volume])
    test('api_template_show_code_for_volume' + volume + '_with_parameters_' + pars.replace(' ', ''),
         python_exe,
         is_parallel: false,
         env: test_env,
         args : [system_template, '-gv', volume, '-gvp',  pars.replace(' ', '')])
endforeach

test('api_show_template_code_for_G4Cons',
     python_exe,
     is_parallel: false,
     env: test_env,
     args : [system_template, '-gv', 'G4Cons', '-silent'])

test_dir = join_paths(build_root, 'test')
# Ensure the per-volume test directory exists in a clean build tree (CI/Linux).
run_command('mkdir', '-p', test_dir, check: true)

test('api_create_system',
     python_exe,
     is_parallel: false,
     env: test_env,
     args : [system_template, '-s', 'test'],
     workdir : build_root,
     priority : 5)
foreach format : ['ascii', 'sqlite']
    test('api_template_build_geometry_in_test_for_format_' + format,
         python_exe,
         is_parallel: false,
         env: test_env,
         args : ['test.py', '-f', format],
         workdir : test_dir,
         priority : -1)
    test('api_run_gemc_for_format_' + format,
         gemc, args : ['test.yaml', '-gsystem="[{name: test, factory: ' + format + '}]" '],
         workdir : test_dir,
         is_parallel: false,
         env : project_test_env,
         priority : -2)
endforeach

# replace geometry with templates
foreach volume: g4objects.keys()
    test_name='test_' + volume
    yaml_file = test_name + '.yaml'
    python_main = test_name + '.py'
    sub_name='-geo_sub=build_' + test_name
    geo_py = '-write_to=geometry_test_' + volume + '.py'
    test_dir = join_paths(build_root, test_name)
    # Ensure the per-volume test directory exists in a clean build tree (CI/Linux).
    run_command('mkdir', '-p', test_dir, check: true)

    pars = g4objects[volume]
    test('api_create_template_system_with' + volume,
         python_exe,
         is_parallel: false,
         env: test_env,
         args : [system_template, '-s', test_name],
         workdir : build_root,
         priority : 4)
    test('api_template_replace_geometry_with' + volume,
         python_exe,
         is_parallel: false,
         env: test_env,
         args: [system_template, '-gv', volume, '-gvp', pars, geo_py, sub_name],
         workdir : build_root,
         priority : -3)
    foreach format : ['ascii', 'sqlite']
        test('api_template_build_geometry_in_test_of_replacing_geometry_with' + volume + '_with_format' + format,
             python_exe,
             is_parallel: false,
             env: test_env,
             args : [python_main, '-f', format],
             workdir : test_dir,
             priority : -4)
        test('api_run_gemc_with_replaced_geometry_using_' + volume + '_with_format' + format,
             gemc,
             args : [yaml_file, '-gsystem="[{name: ' + test_name + ', factory: ' + format + '}]" '],
             workdir : test_dir,
             is_parallel: false,
             env : project_test_env,
             priority : -30)
    endforeach
endforeach
