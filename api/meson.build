api_dir = get_option('prefix') + '/api/'
python_exe = import('python').find_installation('python3')

install_subdir(meson.current_source_dir(), install_dir : '.')

test('api_template_help',
     python_exe,
     env : {'PYTHONDONTWRITEBYTECODE': '1'},
     args : [api_dir + 'system_template.py', '-h'])

test('api_show_solid_creators',
     python_exe,
     env : {'PYTHONDONTWRITEBYTECODE': '1'},
     args : [api_dir + 'system_template.py', '-sl'])

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
         env : {'PYTHONDONTWRITEBYTECODE': '1'},
         args : [api_dir + 'system_template.py', '-gv', volume])
    test('api_template_show_code_for_volume' + volume + '_with_parameters_' + pars.replace(' ', ''),
         python_exe,
         is_parallel: false,
         env : {'PYTHONDONTWRITEBYTECODE': '1'},
         args : [api_dir + 'system_template.py', '-gv', volume, '-gvp',  pars.replace(' ', '')])
endforeach

test('api_show_template_code_for_G4Cons',
     python_exe,
     is_parallel: false,
     env : {'PYTHONDONTWRITEBYTECODE': '1'},
     args : [api_dir + 'system_template.py', '-gv', 'G4Cons', '-silent'])

test_dir = meson.current_build_dir() + '/../test'
test('api_create_system',
     python_exe,
     is_parallel: false,
     env : {'PYTHONDONTWRITEBYTECODE': '1'},
     args : [api_dir + 'system_template.py', '-s', 'test'], priority : 5)
foreach format : ['ascii', 'sqlite']
    test('api_template_build_geometry_in_test_for_format_' + format,
         python_exe,
         is_parallel: false,
         env : {'PYTHONDONTWRITEBYTECODE': '1'},
         args : ['test.py', '-f', format], workdir : test_dir, priority : -1)
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
    test_dir = meson.current_build_dir() + '/../' + test_name
    pars = g4objects[volume]
    test('api_create_template_system_with' + volume,
         python_exe,
         is_parallel: false,
         env : {'PYTHONDONTWRITEBYTECODE': '1'},
         args : [api_dir + 'system_template.py', '-s', test_name], priority : 4)
    test('api_template_replace_geometry_with' + volume,
         python_exe,
         is_parallel: false,
         env : {'PYTHONDONTWRITEBYTECODE': '1'},
         args: [api_dir + 'system_template.py', '-gv', volume, '-gvp', pars, geo_py, sub_name], workdir : test_dir, priority : -3)
    foreach format : ['ascii', 'sqlite']
        test('api_template_build_geometry_in_test_of_replacing_geometry_with' + volume + '_with_format' + format,
             python_exe,
             is_parallel: false,
             env : {'PYTHONDONTWRITEBYTECODE': '1'},
             args : [python_main, '-f', format], workdir : test_dir, priority : -4)
        test('api_run_gemc_with_replaced_geometry_using_' + volume + '_with_format' + format,
             gemc,
             args : [yaml_file, '-gsystem="[{name: ' + test_name + ', factory: ' + format + '}]" '],
             workdir : test_dir,
             is_parallel: false,
             env : project_test_env,
             priority : -30)
    endforeach
endforeach
