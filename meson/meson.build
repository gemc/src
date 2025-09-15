# qt
qt6 = import('qt6')
qt6_deps = dependency('qt6',
                      modules : ['Core', 'Gui', 'Widgets', 'OpenGLWidgets'],
                      static : true,
                      include_type : 'preserve')

# check system: pkg-config --cflags --libs gl
ogl_deps = dependency('opengl', required : true)

# pkg-config --cflags --libs gl.
if host_machine.system() == 'darwin'
    sqlite_dep = dependency('sqlite3', required : true)
else
    # no static or it will link to static system
    sqlite_dep = dependency('sqlite3', static : false, required : true)
endif


# pkg-config --modversion expat
expat_dep = dependency('expat', required : true)

# pkg-config --cflags --libs zlib
zlib_dep = dependency('zlib', static : false, required : true)  # Use shared libz.so instead of libz.a

# clhep: pkg-config
clhep_deps = dependency('clhep',
                        version : '>=2.4.7.1',
)

# install root.pc, geant4.pc if not found
fs = import('fs')
gemc_prefix = get_option('prefix')
geant4_pc = gemc_prefix / 'lib/pkgconfig/geant4.pc'
script_path = meson.project_source_root() / 'meson/install_geant4_root_pkgconfig.py'
if not fs.exists(geant4_pc)
    message('geant4.pc not found in ' + geant4_pc + ', running install script...')
    run_command(script_path, gemc_prefix, check : true)
endif

# root: pkg-config created manually by ./meson/install_geant4_root_pkgconfig.py
rootcfg = find_program('root-config')
if rootcfg.found()
    root_deps = dependency('root',
                           version : '>=6.28/02',
    )
endif
#root_deps = dependency('root', method: 'cmake')

# geant4: pkg-config created manually by ./meson/install_geant4_root_pkgconfig.py
#geant4_deps = dependency('geant4',
#                         version : '>=11.3.0',
#)
# this also works,but ONLY if the dynamic libraries are present
# geant4_deps = dependency('Geant4', method : 'cmake', static : true)

g4_all_modules = [
    # Shared-only in your install:
    'Geant4::G4tools',

    # Static variants available (from your valid target list):
    'Geant4::G4ptl-static',
    'Geant4::G4analysis-static',
    'Geant4::G4digits_hits-static',
    'Geant4::G4error_propagation-static',
    'Geant4::G4event-static',
    'Geant4::G4geometry-static',
    'Geant4::G4global-static',
    'Geant4::G4graphics_reps-static',
    'Geant4::G4intercoms-static',
    'Geant4::G4interfaces-static',
    'Geant4::G4materials-static',
    'Geant4::G4parmodels-static',
    'Geant4::G4particles-static',
    'Geant4::G4geomtext-static',
    'Geant4::G4mctruth-static',
    'Geant4::G4gdml-static',
    'Geant4::G4physicslists-static',
    'Geant4::G4processes-static',
    'Geant4::G4readout-static',
    'Geant4::G4run-static',
    'Geant4::G4track-static',
    'Geant4::G4tracking-static',
    'Geant4::G4FR-static',
    'Geant4::G4visHepRep-static',
    'Geant4::G4RayTracer-static',
    'Geant4::G4Tree-static',
    'Geant4::G4VRML-static',
    'Geant4::G4GMocren-static',
    'Geant4::G4vis_management-static',
    'Geant4::G4modeling-static',
    'Geant4::G4ToolsSG-static',
    'Geant4::G4OpenGL-static',
    'Geant4::G4visQt3D-static',
]

geant4_deps = dependency(
    'Geant4',
    method: 'cmake',
    version: '>=11.3.0',
   # modules: g4_all_modules,
    static: true,      # prefer static libraries when available
    required: true,
)



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
yaml_cpp_dep = yaml_cpp_proj.dependency('yaml_cpp')

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
