examples_dir = get_option('prefix') + '/examples/'
run_command('rm', '-rf', examples_dir)  # Remove dir first
python_exe = import('python').find_installation('python3')

install_subdir(meson.current_source_dir(), install_dir : '.')

# Test all solid creators
examples_map = {
    'geant4_basic' : 'B1'
}

foreach branch : examples_map.keys()
    examples = examples_map[branch].split(' ')

    foreach example : examples
        example_dir = examples_dir + branch + '/' + example + '/'
        yaml_file = example_dir + example + '.yaml'
        test(' ## Building example' + example_dir + ' geometry ##',
             python_exe,
             args : [example_dir + example + '.py'],
             workdir : example_dir,
             env : {'PYTHONDONTWRITEBYTECODE': '1'},
             priority : 10)
        test(' ## run gemc on example ' + example_dir,
             gemc,
             args : [yaml_file],
             env : project_test_env,
             workdir : test_dir,
             priority : -10)
    endforeach

endforeach
