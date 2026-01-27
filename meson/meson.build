# qt
qt6 = import('qt6')
qt6_deps = dependency('qt6',
                      modules : ['Core', 'Gui', 'Widgets', 'OpenGLWidgets', 'Svg'],
                      static : true,
                      include_type : 'system') # treat as system include paths - no warnings

# check system: pkg-config --cflags --libs gl
ogl_deps = dependency('opengl', required : true)

sys = host_machine.system()

# pkg-config --cflags --libs gl.
if sys == 'darwin'
    sqlite_dep = dependency('sqlite3', required : true)
else
    # no static or it will link to static system
    sqlite_dep = dependency('sqlite3', static : false, required : true)
endif


# pkg-config --cflags --libs zlib
zlib_dep = dependency('zlib', static : false, required : true)  # Use shared libz.so instead of libz.az

# clhep: pkg-config
clhep_deps = dependency('clhep',
                        version : '>=2.4.7.1',
                        include_type : 'system') # treat as system include paths - no warnings

# expat required as geant4 is linked statically
# Geant4’s GIDI/XML parsing in G4processes uses Expat
expat_dep = dependency('expat', required: true)


# feature: feature toggle (uncomment options() above)
root_opt = get_option('root')
root_dep = disabler() # Always define root_dep so it exists even if not found

# Only try to add ROOT if not disabled and root-config was found
if (not root_dep.found()) and (not root_opt.disabled())
    # Probe root-config once
    rootcfg = find_program('root-config', required : false)
    if rootcfg.found()
#        root_dep = dependency('ROOT',
#                              version : '>=6.32.16',
#                              method : 'cmake',
#                              modules : ['ROOT::Core', 'ROOT::RIO', 'ROOT::Tree', 'ROOT::Hist'],
#                              required : false,
#                              include_type : 'system') # treat as system include paths - no warnings

        root_prefix = run_command(rootcfg, '--prefix', check: true).stdout().strip()
        root_libdir = run_command(rootcfg, '--libdir', check: true).stdout().strip()

        root_cmake = join_paths(root_libdir, 'cmake', 'ROOT')
        if not fs.is_dir(root_cmake)
            root_cmake = join_paths(root_prefix, 'lib', 'cmake', 'ROOT')
        endif

        root_dep = dependency('ROOT',
                              version: '>=6.32.16',
                              method: 'cmake',
                              modules: ['ROOT::Core', 'ROOT::RIO', 'ROOT::Tree', 'ROOT::Hist'],
                              cmake_args: ['-DROOT_DIR=' + root_cmake],
                              required: false,
                              include_type: 'system')

        if root_dep.found()
            message('root-config found; ROOT dependency resolved: ' + root_dep.version())
        else
            message('root-config found, but ROOT CMake dependency NOT resolved; ROOT features will be disabled')
        endif
    else
        if root_opt.enabled()
            warning('ROOT was explicitly enabled (-Droot=enabled) but root-config was not found.')
            warning('ROOT-dependent features will be disabled.')
        else
            warning('ROOT not found; ROOT-dependent features will be disabled.')
        endif
    endif
endif

# geant4.pc if not found
fs = import('fs')
gemc_prefix = get_option('prefix')
geant4_pc_path = gemc_prefix / 'lib/pkgconfig'
geant4_core_pc = geant4_pc_path / 'geant4_core.pc'
geant4_pc = geant4_pc_path / 'geant4.pc'
g4_pkgconfig_script = meson.project_source_root() / 'meson/g4_pkgconfig.py'
if not fs.exists(geant4_pc) or not fs.exists(geant4_core_pc)
    message('geant4.pc or geant4_core.pc  not found in ' + geant4_pc_path + ', running install script to install it in ' + gemc_prefix)
    run_command(g4_pkgconfig_script, gemc_prefix, check : true)
endif
geant4_dep = dependency('geant4', version : '>=11.3.2', method : 'pkg-config', include_type : 'system') # treat as system include paths - no warnings

geant4_core_dep = dependency('geant4_core', version : '>=11.3.2', method : 'pkg-config')

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

geant4_deps = [geant4_dep]
geant4_core_deps = [geant4_core_dep]
if sys == 'linux'
    # Only on Linux: pull in GLX/X11 stack for G4OpenGL(.a)
    geant4_deps += [
        dependency('x11', required : true),
        dependency('xmu', required : true),
        dependency('gl', required : true),
    ]
endif



cmake = import('cmake')
cmake_opts = cmake.subproject_options()
cmake_opts.add_cmake_defines({
                                 'BUILD_SHARED_LIBS' : false, # or false if you want static assimp
                                 'BUILD_STATIC_LIBS' : true,
                                 'ZLIB_USE_STATIC_LIBS' : false, # prefer shared zlib
                                 'CMAKE_POSITION_INDEPENDENT_CODE' : true,
                                 'CMAKE_C_STANDARD' : 17, # avoid C23 warnings in bundled zlib if you ever enable it
                                 'CMAKE_POLICY_VERSION_MINIMUM' : '4.0',
                             })

sanitize = get_option('b_sanitize')  # string option: 'none', 'address', 'undefined', etc.

# only use shared libraries if a sanitizer option is passed
# this will avoid duplicate symbols errors when sanitizer is used, coming from static library usage
use_sharedl = (sanitize != 'none')
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


cmake_opts.append_compile_args('cpp', '-Wno-shadow')

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
