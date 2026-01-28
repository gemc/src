sub_dir_name = meson.current_source_dir().split('/').get(-1)
internal_deps = ['goptions', 'guts', 'gtouchable', 'gfactory']

# notice the test run on the installation dir, which does has 'examples' stripped from the path
examples_dir = get_option('prefix') + '/test/' + sub_dir_name
gsystem_sqlite = ['-gsystem="[{name: b1}]"', '-sql=' + examples_dir + '/gemc.db']
gsystem_ascii = ['-gsystem="[{name: b1, factory: ascii}]"', '-ascii_db=' + examples_dir]
example_pysource = files('examples/example.py')
example_source = files('examples/gsystem_example.cc')
verbosity = [
    '-verbosity.gvolume=2',
    '-debug.gvolume=true',
    '-verbosity.gmaterial=2',
    '-debug.gmaterial=true',
    '-verbosity.gsystem=2',
    '-debug.gsystem=true',
    '-verbosity.gworld=2',
    '-debug.gworld=true',
    '-verbosity.gsfactory=2',
    '-debug.gsfactory=true'
]

LD += {
    'name' : sub_dir_name,
    'sources' : files(
        'gvolume.cc',
        'gmaterial.cc',
        'gsystem.cc',
        'gworld.cc',
        'gsystem_options.cc',
        'gsystemFactories/text/systemTextFactory.cc',
        'gsystemFactories/text/loadMaterials.cc',
        'gsystemFactories/text/loadGeometry.cc',
        'gsystemFactories/cad/systemCadFactory.cc',
        'gsystemFactories/cad/loadMaterials.cc',
        'gsystemFactories/cad/loadGeometry.cc',
        'gsystemFactories/gdml/systemGdmlFactory.cc',
        'gsystemFactories/gdml/loadMaterials.cc',
        'gsystemFactories/gdml/loadGeometry.cc',
        'gsystemFactories/sqlite/systemSqliteFactory.cc',
        'gsystemFactories/sqlite/loadMaterials.cc',
        'gsystemFactories/sqlite/loadGeometry.cc'
    ),
    'headers' : files(
        'gvolume.h',
        'gmaterial.h',
        'gsystem.h',
        'gworld.h',
        'gsystem_options.h',
        'gsystemConventions.h',
        'gsystemFactories/systemFactory.h',
        'gsystemFactories/systemDynamicFactory.h',
        'gsystemFactories/text/systemTextFactory.h',
        'gsystemFactories/cad/systemCadFactory.h',
        'gsystemFactories/gdml/systemGdmlFactory.h',
        'gsystemFactories/sqlite/systemSqliteFactory.h',
    ),

    'dependencies' : [yaml_cpp_dep, clhep_deps, geant4_core_deps, sqlite_dep],
    'internal_dependencies' : internal_deps,

    'additional_includes' : ['gsystem',
                             'gsystem/gsystemFactories',
                             'gsystem/gsystemFactories/cad',
                             'gsystem/gsystemFactories/gdml',
                             'gsystem/gsystemFactories/sqlite',
                             'gsystem/gsystemFactories/text'],
    'geo_build' : {
        'test_gsystem_geo_sqlite' : [example_pysource],
        'test_gsystem_geo_ascii' : [example_pysource, ['-f', 'ascii']]
    },
    'examples' : {
        'test_gsystem_sqlite' : [example_source, gsystem_sqlite],
        'test_gsystem_ascii' : [example_source, gsystem_ascii],
        'test_gsystem_sqlite_verbose' : [example_source, gsystem_sqlite + verbosity],
        'test_gsystem_ascii_verbose' : [example_source, gsystem_ascii + verbosity],
    }

}
