tests_dir = get_option('prefix') + '/tests/'
python_exe = import('python').find_installation('python3')

install_subdir(meson.current_source_dir(), install_dir : '.')

test(' ## gemc version ##', gemc, args : ['-v'])

simple_dir = tests_dir + '/simple'
test(' ## Building tests simple system' + tests_dir + ' ##', python_exe, args : [simple_dir + '/simple.py'], workdir : simple_dir,  priority : 10)
test(' ## gemc on simple system, event module 10, verbosity passed by command line', gemc,
     args : ['-n=100', simple_dir + '/simple.yaml', '-em_log=10', '-verbosity=\"[{event: 2}]\"' ], workdir : simple_dir, priority : -10)
