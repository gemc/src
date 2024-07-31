test_dir = meson.current_build_dir() + '/'

test(' ## gemc version ##', gemc, args : ['-v'])
test(' ## gemc on B1, event module 10, verbosity passed by command line', gemc,
     args : ['-n=100', 'examples/basic/B1/B1.yaml', '-em_log=10', '-verbosity=\"[{event: 2}]\"' ], workdir : test_dir, priority : -30)
