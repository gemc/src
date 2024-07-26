api_dir =get_option('prefix') + '/api/'
python_exe = import('python').find_installation('python3')

install_subdir(meson.current_source_dir(), install_dir : '.')

test('templates help',                python_exe, args: [api_dir  + 'templates.py', '-h'])
test('templates show solid creators', python_exe, args: [api_dir  + 'templates.py', '-sl'])

g4objects = ['G4Box', 'G4Tubs', 'G4Cons', 'G4Trd', 'G4TrapRAW', 'G4TrapG']
foreach obj : g4objects
    test('template show code for solid ' + obj, python_exe, args: [api_dir  + 'templates.py', '-gv', obj])
endforeach
