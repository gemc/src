api_dir = get_option('prefix') + '/api/'
run_command('rm', '-rf', api_dir)  # Remove dir first
python_exe = import('python').find_installation('python3')

install_subdir(meson.current_source_dir(), install_dir : '.')

test(' ## templates help',
     python_exe,
     env : {'PYTHONDONTWRITEBYTECODE': '1'},
     args : [api_dir + 'templates.py', '-h'])
test(' ## templates show solid creators',
     python_exe,
     env : {'PYTHONDONTWRITEBYTECODE': '1'},
     args : [api_dir + 'templates.py', '-sl'])

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
    test(' ## template show code for volume ' + volume,
         python_exe,
         env : {'PYTHONDONTWRITEBYTECODE': '1'},
         args : [api_dir + 'templates.py', '-gv', volume])
    test(' ## template show code for volume ' + volume + ' with parameters ' + pars,
         python_exe,
         env : {'PYTHONDONTWRITEBYTECODE': '1'},
         args : [api_dir + 'templates.py', '-gv', volume, '-gvp', pars])
endforeach

test(' ## template show code for solid G4Cons in silent mode',
     python_exe,
     env : {'PYTHONDONTWRITEBYTECODE': '1'},
     args : [api_dir + 'templates.py', '-gv', 'G4Cons', '-silent'])

# general system creator
test_dir = meson.current_build_dir() + '/../test'
test(' ## template create system test',
     python_exe,
     env : {'PYTHONDONTWRITEBYTECODE': '1'},
     args : [api_dir + 'templates.py', '-s', 'test'], priority : 10)
test(' ## template build geometry in test',
     python_exe,
     env : {'PYTHONDONTWRITEBYTECODE': '1'},
     args : ['test.py'], workdir : test_dir, priority : -10)
test(' ## run gemc in test',
     gemc, args : ['test.yaml'],
     workdir : test_dir,
     env : project_test_env,
     priority : -20)

# replace geometry with templates
foreach volume: g4objects.keys()
    test_name='test_' + volume
    yaml_file = test_name + '.yaml'
    python_main = test_name + '.py'
    sub_name='-geo_sub=build_' + test_name
    geo_py = '-write_to=geometry_test_' + volume + '.py'
    test_dir = meson.current_build_dir() + '/../' + test_name
    pars = g4objects[volume]
    test(' ## template create system test for volume' + volume,
         python_exe,
         env : {'PYTHONDONTWRITEBYTECODE': '1'},
         args : [api_dir + 'templates.py', '-s', test_name], priority : 10)
    test(' ## template replace geometry in test with ' + volume,
         python_exe,
         env : {'PYTHONDONTWRITEBYTECODE': '1'},
         args: [api_dir + 'templates.py', '-gv', volume, '-gvp', pars, geo_py, sub_name], workdir : test_dir, priority : -10)
    test(' ## template build geometry in test of replacing geometry with ' + volume,
         python_exe,
         env : {'PYTHONDONTWRITEBYTECODE': '1'},
         args : [python_main], workdir : test_dir, priority : -20)
    test(' ## run gemc in test with replaced geometry using ' + volume,
         gemc,
         args : [yaml_file],
         workdir : test_dir,
         env : project_test_env,
         priority : -30)
endforeach
