sub_dir_name = meson.current_source_dir().split('/').get(-1)
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
    'dependencies' : [yaml_cpp_dep, clhep_deps, geant4_deps],
    'additional_includes' : ['gsystem/gsystemFactories',
                             'gsystem/gsystemFactories/cad',
                             'gsystem/gsystemFactories/gdml',
                             'gsystem/gsystemFactories/sqlite',
                             'gsystem/gsystemFactories/text'],
    'examples' : {
        'test_gsystem' : [files('examples/gsystem_example.cc'), meson.current_source_dir() + '/examples/example.yaml']
    }
}
