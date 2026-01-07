# qt
qt6 = import('qt6')
qt6_deps = dependency('qt6',
                      modules : ['Core', 'Gui', 'Widgets', 'OpenGLWidgets', 'Svg'],
                      static : true,
                      include_type : 'preserve')

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
zlib_dep = dependency('zlib', static : false, required : true)  # Use shared libz.so instead of libz.a

# clhep: pkg-config
clhep_deps = dependency('clhep',
                        version : '>=2.4.7.1',
)


# feature: feature toggle (uncomment options() above)
root_opt = get_option('root')
root_dep = disabler() # Always define root_dep so it exists even if not found

# Only try to add ROOT if not disabled and root-config was found
if (not root_dep.found()) and (not root_opt.disabled())
    # Probe root-config once
    rootcfg = find_program('root-config', required : false)
    if rootcfg.found()
        root_mods = []
        root_dep = dependency('root',
                              version : '>=6.32.16',
                              method : 'cmake',
                              modules : ['ROOT::Core', 'ROOT::RIO', 'ROOT::Tree', 'ROOT::Hist'],
                              required : false
        )
        message( 'root-config found. root_dep  dependency added')
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
geant4_pc = gemc_prefix / 'lib/pkgconfig/geant4.pc'
script_path = meson.project_source_root() / 'meson/g4_pkgconfig.py'
if not fs.exists(geant4_pc)
    message('geant4.pc not found in ' + geant4_pc + ', running install script...')
    run_command(script_path, gemc_prefix, check : true)
endif
geant4_dep = dependency('geant4', version : '>=11.3.0')

geant4_deps = [geant4_dep]
if sys == 'linux'
    # Only on Linux: pull in GLX/X11 stack for G4OpenGL(.a)
    geant4_deps += [
        dependency('x11', required : true),
        dependency('xmu', required : true),
        dependency('gl', required : true),
        dependency('expat', required : true)
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
