# Keep unfinished translations out of QM files so partial catalogs always retain English fallback.
qt_gui_lrelease = find_program(
    'lrelease6', 'lrelease-qt6', '/usr/lib/qt6/bin/lrelease', '/usr/lib64/qt6/bin/lrelease',
    '/opt/homebrew/opt/qt/bin/lrelease', 'lrelease')
qt_gui_catalogs = []
foreach language : ['it', 'ja', 'fr', 'zh']
    qt_gui_catalogs += custom_target(
        'gui_catalog_' + language,
        input: language + '.ts',
        output: language + '.qm',
        command: [qt_gui_lrelease, '-nounfinished', '@INPUT@', '-qm', '@OUTPUT@'],
        build_by_default: true,
        install: true,
        install_dir: get_option('datadir') / 'gemc/translations/gui',
        install_tag: 'i18n')
endforeach
