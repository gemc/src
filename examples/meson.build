examples_dir = get_option('prefix') + '/examples/'
python_exe = import('python').find_installation('python3')

install_subdir(meson.current_source_dir(), install_dir : '.')

# Test all solid creators
examples_map = {
    'geant4_basic' : {
        'b1' : { 'runs' : ['1'], 'variations' : ['default', 'test'] },
        'b2' : { 'runs' : ['1', '11'], 'variations' : ['default', 'alt'] }
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
                ascii_output = '-gstreamer="[{format: ascii, filename: out' + branch + variation + '}]"'
                root_output = '-gstreamer="[{format: root, filename: out' + branch + variation + '}]"'
                test(geo_test_name,
                     python_exe,
                     args : [example_dir + example + '.py', '-f', format, '-sql', '../../gemc.db', '-v', variation], # the sql flag is ignored for ascii format
                     workdir : example_dir,
                     env : { 'PYTHONDONTWRITEBYTECODE' : '1' },
                     priority : 10)
                test(run_test_name_asci,
                     gemc,
                     args : [yaml_file, '-gsystem="[{name: ' + example + ', factory: ' + format + ', variation: ' + variation + '}]"', ascii_output, '-sql=../../gemc.db'],
                     workdir : example_dir,
                     priority : -10)
                test(run_test_name_root,
                     gemc,
                     args : [yaml_file, '-gsystem="[{name: ' + example + ', factory: ' + format + ', variation: ' + variation + '}]"', root_output, '-sql=../../gemc.db'],
                     workdir : example_dir,
                     priority : -15)
            endforeach

            # Geometry building tests (runs loop): Loop over runs with fixed variation "default"
            foreach run_value : runs
                geo_test_name = 'examples_geo_' + branch + '_' + example + '_' + format + '_run_' + run_value
                run_test_name_asci = 'examples_run_asci_output_' + branch + '_' + example + '_' + format + '_run_' + run_value
                run_test_name_root = 'examples_run_root_output_' + branch + '_' + example + '_' + format + '_run_' + run_value
                ascii_output = '-gstreamer="[{format: ascii, filename: out' + branch + run_value + '}]"'
                root_output = '-gstreamer="[{format: root, filename: out' + branch + run_value + '}]"'
                test(geo_test_name,
                     python_exe,
                     args : [example_dir + example + '.py', '-f', format, '-sql', '../../gemc.db', '-r', run_value], # the sql flag is ignored for ascii format
                     workdir : example_dir,
                     env : { 'PYTHONDONTWRITEBYTECODE' : '1' },
                     priority : 5)
                test(run_test_name_asci,
                     gemc,
                     args : [yaml_file, '-gsystem="[{name: ' + example + ', factory: ' + format + ', runno:' + run_value + '}]"', ascii_output, '-sql=../../gemc.db'],
                     workdir : example_dir,
                     priority : -20)
                test(run_test_name_root,
                     gemc,
                     args : [yaml_file, '-gsystem="[{name: ' + example + ', factory: ' + format + ', runno:' + run_value + '}]"', root_output, '-sql=../../gemc.db'],
                     workdir : example_dir,
                     priority : -25)
            endforeach

        endforeach
    endforeach
endforeach
