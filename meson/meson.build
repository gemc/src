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
                                 'OpenGLWidgets',
                                 'Svg'],
                      static : false,
                      include_type : 'system') # treat as system include paths - no warnings
qt6_charts_dep = dependency('qt6',
                            modules : ['Charts'],
                            static : false,
                            include_type : 'system')

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

# Generate Geant4 pkg-config files from the active module so downstream builds
# do not pick stale global pc files from unrelated pkg-config directories.
fs = import('fs')
gemc_prefix = get_option('prefix')
geant4_pc_path = gemc_prefix / 'lib/pkgconfig'
g4_pkgconfig_script = meson.project_source_root() / 'meson/g4_pkgconfig.py'

geant4_config_version = run_command(geant4_config, '--version', check : true).stdout().strip()
message('Generating Geant4 pkg-config files in ' + geant4_pc_path + ' from ' + geant4_config.full_path())
# Configure-time generation: GEMC's own dependency('geant4_core') below needs the .pc files on
# PKG_CONFIG_PATH to build itself.
run_command(g4_pkgconfig_script, gemc_prefix, geant4_config.full_path(), check : true)
# Install-time generation: re-create geant4.pc / geant4_core.pc in the prefix on every install.
# Without this the files exist only as a one-shot configure side effect, so a reinstall into a
# freshly wiped prefix (e.g. rm -rf install_dir + meson install) silently drops them and downstream
# builds such as clas12-systems can no longer resolve geant4_core.
meson.add_install_script(
    find_program('python3'),
    files('g4_pkgconfig.py'),
    gemc_prefix,
    geant4_config.full_path(),
)

geant4_dep = dependency(
    'geant4',
    version      : '>=11.3.2',
    method       : 'pkg-config',
    include_type : 'system',
)
geant4_core_dep = dependency('geant4_core', version : '>=11.3.2', method : 'pkg-config')
if geant4_core_dep.version() != geant4_config_version
    error(
        'pkg-config resolved geant4_core ' + geant4_core_dep.version() +
        ', but active geant4-config reports ' + geant4_config_version +
        '. Put ' + geant4_pc_path + ' before stale pkg-config directories and reconfigure.',
    )
endif
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
