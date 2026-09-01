sanitize = get_option('b_sanitize')  # string option: 'none', 'address', 'undefined', etc.

# only use shared libraries if a sanitizer option is passed
# this will avoid duplicate symbols errors when sanitizer is used, coming from static library usage
use_sharedl = (sanitize != 'none')

# qt
qt6 = import('qt6')
# Supported package managers provide Qt as shared runtime libraries, including for binary tarballs.
# Requesting static Qt archives only produces fallback warnings on Linux and does not make Qt static.
qt6_deps = dependency('qt6',
                      modules : ['Core',
                                 'Gui',
                                 'Widgets',
                                 'OpenGLWidgets'],
                      static : false,
                      include_type : 'system') # treat as system include paths - no warnings

# Qt Svg is NOT a compile/link dependency: no GEMC source includes a QtSvg header or uses a QSvg
# class. It is used only at run time to rasterize the GUI toolbar button icons through the "SVG"
# image-format plugin (QPixmap::loadFromData(svg, "SVG")). We therefore probe it optionally and only
# warn when it is missing: the GUI still builds and runs, but the toolbar buttons render blank icons
# (see gemc/gqtbuttonswidget/gQtButtonsWidget.cc, which already falls back to an empty QIcon).
qt6_svg_dep = dependency('qt6',
                         modules : ['Svg'],
                         static : false,
                         include_type : 'system',
                         required : false)
if not qt6_svg_dep.found()
    warning('Qt6 Svg module not found: GUI toolbar buttons will render without icons. ' +
            'Install the Qt6 Svg package (e.g. qt6-qtsvg-devel) to enable them.')
endif
# Qt Charts is a real compile/link dependency: gemc/ganalysis/gAnalysisView.cc includes
# <QtCharts/...> and links Qt6Charts to draw the live in-GUI histogram page. Unlike Svg it cannot
# degrade at run time, so we probe it optionally: when present we define GEMC_HAS_QTCHARTS and build
# the analysis view; when absent GEMC still builds and runs, but the GUI analysis page is replaced by
# a short "Qt6 Charts required" placeholder (see gemc/gui/rightContent.cc) and the Charts-only
# ganalysis sources/examples are dropped from the build (see gemc/ganalysis/meson.build).
qt6_charts_dep = dependency('qt6',
                            modules : ['Charts'],
                            static : false,
                            include_type : 'system',
                            required : false)
qt6_charts_found = qt6_charts_dep.found()
if qt6_charts_found
    add_project_arguments('-DGEMC_HAS_QTCHARTS', language : 'cpp')
else
    warning('Qt6 Charts module not found: the GUI analysis (histogram) page is disabled and shows a ' +
            'placeholder. Install the Qt6 Charts package (e.g. qt6-qtcharts-devel) to enable it.')
endif

sys = host_machine.system()

# pkg-config --cflags --libs gl
ogl_deps = dependency('opengl', required : true)

# pkg-config --cflags --libs sqlite3
if sys == 'darwin'
    sqlite_dep = dependency('sqlite3', required : true)
else
    # no static or it will link to static system
    sqlite_dep = dependency('sqlite3', static : false, required : true)
endif

# expat required as geant4 is linked statically
# Geant4’s GIDI/XML parsing in G4processes uses Expat
expat_dep = dependency('expat', required: true)

# pkg-config --cflags --libs zlib
zlib_dep = dependency('zlib', static : false, required : true)  # Use shared libz.so instead of libz.az

g4install = run_command('sh', '-c', 'printf %s "$G4INSTALL"', check : false).stdout().strip()
geant4_config = find_program('geant4-config', required : false)
if g4install != ''
    geant4_module_config = find_program(g4install / 'bin' / 'geant4-config', required : false)
    if geant4_module_config.found()
        geant4_config = geant4_module_config
    endif
endif

if not geant4_config.found()
    error('geant4-config was not found. Load a Geant4 module before configuring GEMC.')
endif

# pkg-config --cflags --libs clhep
clhep_deps = dependency(
    'clhep',
    version      : '>=2.4.7.1',
    include_type : 'system',
)

# feature: feature toggle (uncomment options() above)
root_opt = get_option('root')
root_dep = disabler() # Always define root_dep so it exists even if not found

# Only try to add ROOT if not disabled and root-config was found
if (not root_dep.found()) and (not root_opt.disabled())
    # Probe root-config once
    rootcfg = find_program('root-config', required : false)
    if rootcfg.found()

        # This commented method did not work on archlinux
        # However the method below did work
        #        root_prefix = run_command(rootcfg, '--prefix', check: true).stdout().strip()
        #        root_libdir = run_command(rootcfg, '--libdir', check: true).stdout().strip()
        #
        #        root_cmake = join_paths(root_libdir, 'cmake', 'ROOT')
        #        if not fs.is_dir(root_cmake)
        #            root_cmake = join_paths(root_prefix, 'lib', 'cmake', 'ROOT')
        #        endif
        #        message('root_cmake files location:' + root_cmake )
        #
        #        root_dep = dependency('ROOT',
        #                              version: '>=6.32.16',
        #                              method: 'cmake',
        #                              modules : ['Core', 'RIO', 'Tree', 'Hist'],
        #                              cmake_args: ['-DROOT_DIR=' + root_cmake],
        #                              required: false,
        #                              include_type: 'system')

        root_cflags = run_command(rootcfg, '--cflags', check: true).stdout().strip().split()
        root_libs   = run_command(rootcfg, '--libs',   check: true).stdout().strip().split()

        root_dep = declare_dependency(
            compile_args: root_cflags,
            link_args: root_libs,
        )

        if root_dep.found()
            message('root-config found; ROOT dependency resolved: ' + root_dep.version())
        else
            message('root-config found, but ROOT CMake dependency NOT resolved; ROOT features will be disabled')
        endif
    else
        if root_opt.enabled()
            error('ROOT was explicitly enabled (-Droot=enabled) but root-config was not found.')
        else
            warning('ROOT not found; ROOT-dependent features will be disabled.')
        endif
    endif
endif

# Geant4 ships no pkg-config file, so we generate geant4.pc / geant4_core.pc ourselves from the active
# geant4-config. They are a build-time helper for locating Geant4, so they live in a hardcoded
# directory inside the build tree (not tied to the install prefix). A fresh `rm -rf build` drops them,
# so they regenerate cleanly when the build dir is recreated - e.g. after switching Geant4 versions.
fs = import('fs')
geant4_pc_path = meson.project_build_root() / 'pkgconfig'
geant4_pc = geant4_pc_path / 'geant4.pc'
geant4_core_pc = geant4_pc_path / 'geant4_core.pc'
g4_pkgconfig_script = meson.project_source_root() / 'meson/g4_pkgconfig.py'

geant4_config_version = run_command(geant4_config, '--version', check : true).stdout().strip()
if not fs.exists(geant4_pc) or not fs.exists(geant4_core_pc)
    message('Generating geant4.pc / geant4_core.pc in ' + geant4_pc_path + ' from ' + geant4_config.full_path())
    run_command(g4_pkgconfig_script, geant4_pc_path, geant4_config.full_path(), check : true)
endif

# The generated .pc files are not on the default pkg-config search path. Rather than requiring the
# caller to export PKG_CONFIG_PATH / pass -Dpkg_config_path, we resolve geant4 by running pkg-config
# ourselves with an augmented PKG_CONFIG_PATH and wrap the result in declare_dependency: meson.build
# cannot mutate the pkg_config_path option or dependency()'s environment, so this is the faithful
# equivalent of a plain pkg-config lookup. env.append() adds to (does not replace) the inherited
# PKG_CONFIG_PATH, so module-provided paths still resolve. The generated .pc files carry flat
# Cflags/Libs with no Requires, so nothing else needs to be found.
g4_min_version = '>=11.3.2'
pkgconf = find_program('pkg-config', 'pkgconf', required : true)
g4_pc_env = environment()
g4_pc_env.append('PKG_CONFIG_PATH', geant4_pc_path, separator : ':')

g4_built = {}
foreach g4 : [
    { 'pc' : 'geant4',      'system' : true  }, # treat includes as system paths - no warnings
    { 'pc' : 'geant4_core', 'system' : false },
]
    pcname = g4['pc']
    ver = run_command(pkgconf, '--modversion', pcname, env : g4_pc_env, check : true).stdout().strip()
    if not ver.version_compare(g4_min_version)
        error(pcname + ' ' + ver + ' found in ' + geant4_pc_path + ', but ' + g4_min_version + ' is required')
    endif
    if ver != geant4_config_version
        error('pkg-config resolved ' + pcname + ' ' + ver + ', but active geant4-config reports ' +
              geant4_config_version + '. Remove stale entries from PKG_CONFIG_PATH and reconfigure.')
    endif
    cflags = run_command(pkgconf, '--cflags', pcname, env : g4_pc_env, check : true).stdout().split()
    libs = run_command(pkgconf, '--libs', pcname, env : g4_pc_env, check : true).stdout().split()
    compile_args = []
    foreach f : cflags
        if g4['system'] and f.startswith('-I')
            compile_args += ['-isystem', f.substring(2)] # -I<path> -> -isystem <path>
        else
            compile_args += f
        endif
    endforeach
    g4_built += { pcname : declare_dependency(compile_args : compile_args, link_args : libs) }
endforeach
geant4_dep = g4_built['geant4']
geant4_core_dep = g4_built['geant4_core']

# Each static GEMC module library is built with the Geant4 link_args it needs, so a target that links
# several of them lists the same -lG4* more than once. Meson does not deduplicate raw declare_dependency
# link_args (unlike real pkg-config dependency objects), and the Geant4 .pc files are resolved here as
# declare_dependency, so the repeats reach the link line. They are harmless - the linker keeps the first,
# order-correct copy - but Apple's linker warns on every one of them. GNU ld (Linux/ifarm) is silent, so
# quiet the macOS linker to keep the build log readable.
if sys == 'darwin'
    add_project_link_arguments('-Wl,-no_warn_duplicate_libraries', language : 'cpp')
endif

# Ship the generated .pc into <prefix>/lib/pkgconfig. Downstream GEMC projects (e.g. clas12-systems)
# resolve geant4_core straight from the installed GEMC's lib/pkgconfig, so the installed tree must
# carry these files. An install script (rather than install_data) is used because the .pc are
# generated build-tree files; $MESON_INSTALL_DESTDIR_PREFIX is set by meson at install time.
meson.add_install_script(
    'sh', '-c',
    'set -e; d="$MESON_INSTALL_DESTDIR_PREFIX/$1"; mkdir -p "$d"; shift; cp "$@" "$d"',
    'install_geant4_pc', get_option('libdir') / 'pkgconfig', geant4_pc, geant4_core_pc,
)

geant4_deps = [geant4_dep]
geant4_core_deps = [geant4_core_dep]
if sys == 'linux'
    # Only on Linux: pull in GLX/X11 stack for G4OpenGL(.a)
    # Force shared so static non-PIC archives (libX11.a on aarch64) are not linked into shared libs
    geant4_deps += [
        dependency('x11', static : false, required : true),
        dependency('xmu', static : false, required : true),
        dependency('gl', static : false, required : true),
    ]
endif

# cmake template - but still not working
#geant4_dep = dependency(
#    'Geant4',
#    method: 'cmake',
#    version: '>=11.3.0',
#    modules: [
#        'Geant4::G4run-static',
#        'Geant4::G4vis_management-static',
#        'Geant4::G4interfaces-static',
#        'Geant4::G4OpenGL-static',
#    ],
#)



cmake = import('cmake')
cmake_opts = cmake.subproject_options()
cmake_opts.add_cmake_defines({
                                 'BUILD_STATIC_LIBS' : true,
                                 'ZLIB_USE_STATIC_LIBS' : false, # prefer shared zlib
                                 'CMAKE_POSITION_INDEPENDENT_CODE' : true,
                                 # Avoid C23 warnings in bundled zlib if you ever enable it.
                                 'CMAKE_C_STANDARD' : 17,
                                 'CMAKE_POLICY_VERSION_MINIMUM' : '4.0',
                             })

cmake_opts.add_cmake_defines({
                                 'BUILD_SHARED_LIBS' : use_sharedl,
                                 # Avoid pulling GUI/sample/tooling deps like X11
                                 'ASSIMP_BUILD_ASSIMP_TOOLS' : false,
                                 'ASSIMP_BUILD_SAMPLES' : false,
                                 'ASSIMP_BUILD_TESTS' : false,
                             })


if host_machine.system() == 'darwin'
    # there's a macro collision in Assimp’s bundled zlib on macOS.
    # Using system zlib is fine on macos
    cmake_opts.add_cmake_defines({
                                     'ASSIMP_BUILD_ZLIB' : false,
                                 })
else
    # Ubuntu: zlib is not compiled with fPic on the system. Let Assimp compile it.
    cmake_opts.add_cmake_defines({
                                     'ASSIMP_BUILD_ZLIB' : true,
                                 })
endif


# TODO: remove temp workaround for _LIBCPP_ENABLE_ASSERTIONS deprecation
cmake_opts.append_compile_args('cpp',
                               '-fPIC',
                               '-Wno-shadow',
                               '-U_LIBCPP_ENABLE_ASSERTIONS',
                               '-D_LIBCPP_HARDENING_MODE_DEBUG=true',
)

assimp_proj = cmake.subproject('assimp', options : cmake_opts)
assimp_dep = assimp_proj.dependency('assimp')

yaml_cpp_proj = cmake.subproject('yaml-cpp', options : cmake_opts)
yaml_cpp_dep = yaml_cpp_proj.dependency('yaml-cpp')

# yaml-cpp headers are part of gemc's public API (goption.h includes yaml-cpp/yaml.h)
# so they must be installed alongside the gemc headers.
install_subdir(
    meson.project_source_root() / 'subprojects' / 'yaml-cpp' / 'include' / 'yaml-cpp',
    install_dir : get_option('includedir'),
)

# sanitizer settings
project_test_env = environment()
project_test_env.set(
    'UBSAN_OPTIONS',
    'halt_on_error=1',
    'abort_on_error=1',
    'print_summary=1',
    'print_stacktrace=1',
    'verbosity=10',
    'suppressions=' + meson.project_source_root() / 'meson' / 'ubsan.supp',
)
project_test_env.set(
    'ASAN_OPTIONS',
    'halt_on_error=1',
    'abort_on_error=1',
    'print_summary=1',
    'verbosity=10',
    'suppressions=' + meson.project_source_root() / 'meson' / 'asan.supp',
)

project_test_env.set(
    'LSAN_OPTIONS',
    'verbosity=10',
    'suppressions=' + meson.project_source_root() / 'meson' / 'lsan.supp',
)

# allocator_may_return_null=1: Allows TSan to return NULL on allocation failure instead of crashing,
# which might give you more information about where the allocation problem is happening.
project_test_env.set(
    'TSAN_OPTIONS',
    'verbosity=10',
    'heap_size=512M',
    'allocator_may_return_null=1',
    'suppressions=' + meson.project_source_root() / 'meson' / 'tsan.supp',
)
