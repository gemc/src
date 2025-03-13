examples_dir = get_option('prefix') + '/examples/'
run_command('rm', '-rf', examples_dir, check: true)  # Remove dir first
python_exe = import('python').find_installation('python3')

install_subdir(meson.current_source_dir(), install_dir : '.')

# Test all solid creators
examples_map = {
    'geant4_basic' : {
        'b1': {'runs': ['1'], 'variations': ['default', 'test']},
        'b2': {'runs': ['1', '11'], 'variations': ['default', 'alt']}
    }
}

foreach branch : examples_map.keys()
    examples = examples_map[branch]

    foreach example, config : examples
        example_dir = examples_dir + branch + '/' + example + '/'
        yaml_file = example_dir + example + '.yaml'
        runs = config['runs']
        variations = config['variations']

        # Iterate over the formats 'ascii' and 'sqlite'
        foreach format : ['ascii', 'sqlite']
            # Geometry building tests (variations loop): Always use run "1"
            foreach variation : variations
                test(' ## Building example ' + example_dir + ' geometry with format ' + format + ', default run and variation ' + variation + ' ##',
                     python_exe,
                     args : [example_dir + example + '.py', '-f', format, '-sql', '../../gemc.db', '-v', variation],
                     workdir : example_dir,
                     env : {'PYTHONDONTWRITEBYTECODE': '1'},
                     priority : 10)
            endforeach

            # Geometry building tests (runs loop): Loop over runs with fixed variation "default"
            foreach run_value : runs
                test(' ## Building example ' + example_dir + ' geometry with format ' + format + ', run ' + run_value + ' and variation default ##',
                     python_exe,
                     args : [example_dir + example + '.py', '-f', format, '-sql', '../../gemc.db', '-r', run_value],
                     workdir : example_dir,
                     env : {'PYTHONDONTWRITEBYTECODE': '1'},
                     priority : 10)
            endforeach

            # Gemc run tests: Loop over runs with fixed variation "default"
            foreach variation : variations
                test(' ## run gemc on example ' + example_dir + ' with format ' + format + ', default run  and variation default ##',
                     gemc,
                     args : [yaml_file, '-gsystem="[{name:' + example + ', factory: ' + format + ', variation:' +  variation + ', sql_file: ../../gemc.db}]"'],
                     env : project_test_env,
                     workdir : test_dir,
                     priority : -10)
            endforeach
            # Gemc run tests: Loop over runs with fixed variation "default"
            foreach run_value : runs
                test(' ## run gemc on example ' + example_dir + ' with format ' + format + ', run ' + run_value + ' and variation default ##',
                     gemc,
                     args : [yaml_file, '-gsystem="[{name:' + example + ', factory: ' + format + ', runno:' +  run_value + ',  sql_file: ../../gemc.db}]"'],
                     env : project_test_env,
                     workdir : test_dir,
                     priority : -10)
            endforeach
        endforeach
    endforeach
endforeach
