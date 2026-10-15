sub_dir_name = meson.current_source_dir().split('/').get(-1)
internal_deps = ['guts']

# Catalog loading is available during option/startup parsing, before GLogger exists.
catalog_deps = [yaml_cpp_dep, clhep_deps, geant4_core_deps]
catalog_platform_deps = []
if host_machine.system() == 'darwin'
    catalog_platform_deps += dependency('appleframeworks', modules: ['CoreFoundation'])
endif
catalog_deps += catalog_platform_deps

catalog_configuration = configuration_data()
catalog_configuration.set_quoted('GEMC_CATALOG_DIRECTORY', get_option('datadir') / 'gemc/translations')
catalog_configuration.set_quoted('GEMC_CATALOG_BUILD_DIRECTORY', meson.project_build_root())
catalog_configuration.set_quoted('GEMC_CATALOG_SOURCE_DIRECTORY', meson.project_source_root() / 'translations')
configure_file(output: 'gloggingConfig.h', configuration: catalog_configuration)

run_command('version.py', check : true)
example_source = files('examples/define_options.cc')
optional_scalar_source = files('examples/optional_scalar.cc')


LD += {
    'name' : sub_dir_name,
    'sources' : files(
        'goption.cc',
        'goptions.cc',
        '../glogging/gmessageCatalog.cc'
    ),
    'additional_includes' : ['gemc/goptions'],
    'headers' : files(
        'gswitch.h',
        'goption.h',
        'goptions.h',
        'gversion.h',
        'goptionsConventions.h'
    ),
    'dependencies' : catalog_deps,
    'internal_dependencies' : internal_deps,

    'examples' : {
        'test_goptions_definitions' : [example_source, ''],
        'test_goptions_help' : [example_source, '-h'],
        'test_goptions_parse_file' : [example_source, meson.current_source_dir() + '/examples/simple.yaml'],
        'test_goptions_optional_scalar_absent' : [optional_scalar_source, ''],
        'test_goptions_optional_scalar_empty' : [optional_scalar_source, '-optional_string='],
        'test_goptions_optional_scalar_present' : [optional_scalar_source, '-optional_string=configured'],
		'test_goptions_optional_scalar_int' : [optional_scalar_source, '-optional_int=42'],
    }
}
