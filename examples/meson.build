examples_dir = get_option('prefix') + '/examples/'

test_outdir = meson.current_build_dir()
sql_db = get_option('prefix') + '/examples/gemc.db'

install_subdir(meson.current_source_dir(), install_dir : '.')

gemc_analyzer = find_program('gemc-analyzer', dirs : [venv_bin])

# Test all solid creators.
# Optional keys:
#   plot_var  – variable name to histogram from the digitized stream (ascii, default variation only)
#   plot_yvsx – if true, plot avgx vs avgy from the true-info stream (ascii, default variation only)
examples_map = {
    'basic' : {
        'simple_flux' : { 'runs' : ['1'], 'variations' : ['default'], 'plot_var' : 'totEdep' },
        'b1' : { 'runs' : ['1'], 'variations' : ['default', 'test'], 'plot_var' : 'dose', 'n_events' : '50' },
        'b2' : { 'runs' : ['1', '11'], 'variations' : ['default', 'alt'], 'plot_var' : 'totEdep' },
        'b3' : { 'runs' : ['1', '11'], 'variations' : ['default', 'alt'] }
    },
    'optical' : {
        'cherenkov' : { 'runs' : ['1'], 'variations' : ['default', 'mediumIndexRadiator', 'highIndexRadiator'],
                        'plot_yvsx' : true },
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
                     env : test_env,
                     priority : 10)
                test(run_test_name_asci,
                     gemc,
                     args : [yaml_file, '-gsystem="[{name: ' + example + ', factory: ' + format + ', variation: ' + variation + '}]"', ascii_output, '-sql=' + sql_db],
                     workdir : example_dir,
                     is_parallel : false,
                     priority : -10)
                if root_dep.found()
                    test(run_test_name_root,
                         gemc,
                         args : [yaml_file, '-gsystem="[{name: ' + example + ', factory: ' + format + ', variation: ' + variation + '}]"', root_output, '-sql=' + sql_db],
                         workdir : example_dir,
                         is_parallel : false,
                         priority : -11)
                endif
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
                     env : test_env,
                     priority : 9)
                test(run_test_name_asci,
                     gemc,
                     args : [yaml_file, '-gsystem="[{name: ' + example + ', factory: ' + format + ', runno:' + run_value + '}]"', ascii_output, '-sql=' + sql_db],
                     workdir : example_dir,
                     is_parallel : false,
                     priority : -12)
                if root_dep.found()
                    test(run_test_name_root,
                         gemc,
                         args : [yaml_file, '-gsystem="[{name: ' + example + ', factory: ' + format + ', runno:' + run_value + '}]"', root_output, '-sql=' + sql_db],
                         workdir : example_dir,
                         is_parallel : false,
                         priority : -13)
                endif
            endforeach

        endforeach

        # CSV output run + plot tests — one dedicated gemc run per example with a csv streamer,
        # placed after all ascii/root runs (priority -15) so geometry is already built.
        plot_var = config.get('plot_var', '')
        do_yvsx  = config.get('plot_yvsx', false)
        if plot_var != '' or do_yvsx
            csv_base   = test_outdir + '/' + branch + '_' + example + '_csv'
            n_events   = config.get('n_events', '')
            n_events_arg = n_events != '' ? ['-n=' + n_events] : []
            test('examples_run_csv_' + branch + '_' + example,
                 gemc,
                 args : [yaml_file,
                         '-gsystem="[{name: ' + example + ', factory: ascii}]"',
                         '-gstreamer="[{format: csv, filename: ' + csv_base + '}]"'] + n_events_arg,
                 workdir : example_dir,
                 is_parallel : false,
                 priority : -15)
            if plot_var != ''
                test('examples_plot_' + branch + '_' + example,
                     gemc_analyzer,
                     args : [csv_base + '_t0_digitized.csv', plot_var,
                             '--save', test_outdir + '/' + branch + '_' + example + '_plot.png'],
                     workdir : example_dir,
                     is_parallel : false,
                     priority : -20)
            endif
            if do_yvsx
                test('examples_plot_yvsx_' + branch + '_' + example,
                     gemc_analyzer,
                     args : [csv_base + '_t0_true_info.csv',
                             '--kind', 'csv', '--data', 'true_info', '--plot', 'yvsx',
                             '--xlim', '-55', '55', '--ylim', '-55', '55',
                             '--save', test_outdir + '/' + branch + '_' + example + '_yvsx.png'],
                     workdir : example_dir,
                     is_parallel : false,
                     priority : -20)
            endif
        endif
    endforeach
endforeach
