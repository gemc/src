api_dir =get_option('prefix') + '/api/'
python_exe = import('python').find_installation('python3')

install_subdir(meson.current_source_dir(), install_dir : '.')

test(' ## templates help',                python_exe, args: [api_dir  + 'templates.py', '-h'])
test(' ## templates show solid creators', python_exe, args: [api_dir  + 'templates.py', '-sl'])

# Test all solid creators
g4objects = ['G4Box', 'G4Tubs', 'G4Cons', 'G4Trd', 'G4TrapRAW', 'G4TrapG']
foreach obj : g4objects
    test(' ## template show code for solid ' + obj, python_exe, args: [api_dir  + 'templates.py', '-gv', obj])
endforeach
test(' ## template show code for solid G4Cons in silent mode', python_exe, args: [api_dir  + 'templates.py', '-gv', 'G4Cons'])

# Test system creator
test_dir = meson.current_build_dir() + '/../test'
test(' ## template create system test',     python_exe, args: [api_dir  + 'templates.py', '-s', 'test'])
test(' ## template build geometry in test', python_exe, args: [ 'test.py'],  workdir: test_dir, priority : -10 )
test(' ## run gemc in test',                gemc,   args: ['test.yaml'], workdir: test_dir, priority : -20 )
