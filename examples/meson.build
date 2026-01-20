examples_dir = get_option('prefix') + '/examples/'
test_env = environment()
test_env.set('PYTHONDONTWRITEBYTECODE', '1')
test_env.prepend('PYTHONPATH', api_dir)

test_outdir = meson.current_build_dir()
sql_db = get_option('prefix') + '/examples/gemc.db'

python_exe = import('python').find_installation('python3')

install_subdir(meson.current_source_dir(), install_dir : '.')

# Test all solid creators
examples_map = {
    'basic' : {
        'simple_flux' : { 'runs' : ['1'], 'variations' : ['default'] },
        'b1' : { 'runs' : ['1'], 'variations' : ['default', 'test'] },
        'b2' : { 'runs' : ['1', '11'], 'variations' : ['default', 'alt'] },
        'b3' : { 'runs' : ['1', '11'], 'variations' : ['default', 'alt'] }
    }
}

foreach branch : examples_map.keys()
    examples = examples_map[branch]

    foreach example, config : examples
        example_dir = examples_dir + branch + '/' + example + '/'
        yaml_file = example_dir + example + '.yaml'
        runs = config['runs']
        variations = config['variations']

        # Iterate over the database format 'ascii' and 'sqlite'
        foreach format : ['ascii', 'sqlite']
            # Geometry building tests (variations loop): Always use run "1"
            foreach variation : variations
                # form a name string with example_dir and format
                geo_test_name = 'examples_geo_' + branch + '_' + example + '_' + format + '_variation_' + variation
                run_test_name_asci = 'examples_run_asci_output_' + branch + '_' + example + '_' + format + '_variation_' + variation
                run_test_name_root = 'examples_run_root_output_' + branch + '_' + example + '_' + format + '_variation_' + variation
                ascii_output = '-gstreamer="[{format: ascii, filename: ' + test_outdir + '/' + branch + variation + format + '}]"'
                root_output = '-gstreamer="[{format: root, filename: ' + test_outdir + '/' + branch + variation + format + '}]"'
                test(geo_test_name,
                     python_exe,
                     args : [example_dir + example + '.py', '-f', format, '-sql', sql_db, '-v', variation], # the sql flag is ignored for ascii format
                     workdir : example_dir,
                     is_parallel : false,
                     env: test_env,
                     priority : 10)
                test(run_test_name_asci,
                     gemc,
                     args : [yaml_file, '-gsystem="[{name: ' + example + ', factory: ' + format + ', variation: ' + variation + '}]"', ascii_output, '-sql=' + sql_db],
                     workdir : example_dir,
                     is_parallel : false,
                     priority : -10)
                test(run_test_name_root,
                     gemc,
                     args : [yaml_file, '-gsystem="[{name: ' + example + ', factory: ' + format + ', variation: ' + variation + '}]"', root_output, '-sql=' + sql_db],
                     workdir : example_dir,
                     is_parallel : false,
                     priority : -11)
            endforeach

            # Geometry building tests (runs loop): Loop over runs with fixed variation "default"
            foreach run_value : runs
                geo_test_name = 'examples_geo_' + branch + '_' + example + '_' + format + '_run_' + run_value
                run_test_name_asci = 'examples_run_asci_output_' + branch + '_' + example + '_' + format + '_run_' + run_value
                run_test_name_root = 'examples_run_root_output_' + branch + '_' + example + '_' + format + '_run_' + run_value
                ascii_output = '-gstreamer="[{format: ascii, filename: ' + test_outdir + '/' + branch + run_value + format + '}]"'
                root_output = '-gstreamer="[{format: root, filename: ' + test_outdir + '/' + branch + run_value + format + '}]"'
                test(geo_test_name,
                     python_exe,
                     args : [example_dir + example + '.py', '-f', format, '-sql', sql_db, '-r', run_value], # the sql flag is ignored for ascii format
                     workdir : example_dir,
                     is_parallel : false,
                     env: test_env,
                     priority : 9)
                test(run_test_name_asci,
                     gemc,
                     args : [yaml_file, '-gsystem="[{name: ' + example + ', factory: ' + format + ', runno:' + run_value + '}]"', ascii_output, '-sql=' + sql_db],
                     workdir : example_dir,
                     is_parallel : false,
                     priority : -12)
                test(run_test_name_root,
                     gemc,
                     args : [yaml_file, '-gsystem="[{name: ' + example + ', factory: ' + format + ', runno:' + run_value + '}]"', root_output, '-sql=' + sql_db],
                     workdir : example_dir,
                     is_parallel : false,
                     priority : -13)
            endforeach

        endforeach
    endforeach
endforeach
