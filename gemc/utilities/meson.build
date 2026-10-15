sub_dir_name = meson.current_source_dir().split('/').get(-1)
internal_deps = ['glogging']

gui_qresource = configure_file(input: 'gemc_gui_translations.qrc',
                              output: 'gemc_gui_translations.qrc', copy: true)
gui_translation_resources = qt6.compile_resources(name: 'gemc_gui_translations', sources: gui_qresource)

LD += {
    'name' : sub_dir_name,
    'sources' : files(
        'gemcUtilities.cc',
        'gQtTranslation.cc',
    ) + gui_translation_resources,
    'headers' : files(
        'gemcUtilities.h',
        'gemcConventions.h',
        'gQtTranslation.h',
    ),
    'dependencies' : [yaml_cpp_dep, clhep_deps, qt6_deps, geant4_core_deps],
    'internal_dependencies' : internal_deps,
}
