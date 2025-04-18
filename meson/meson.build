# qt
qt6 = import('qt6')
qt6_deps = dependency('qt6',
                      modules : ['Core', 'Gui', 'Widgets', 'OpenGLWidgets'],
                      static : true,
                      include_type : 'preserve')

# check system: pkg-config --cflags --libs gl
ogl_deps = dependency('opengl', required: true)

# pkg-config --cflags --libs gl
sqlite_dep = dependency('sqlite3', required: true)

# pkg-config --modversion expat
expat_dep = dependency('expat', required: true)

# pkg-config --cflags --libs zlib
zlib_dep = dependency('zlib', static: false, required: true)  # Use shared libz.so instead of libz.a

# clhep: pkg-config
clhep_deps = dependency('clhep',
                        version : '>=2.4.7.1',
                        include_type : 'preserve'
)

# install root.pc, geant4.pc if not found
python_exe = import('python').find_installation('python3')
fs = import('fs')
geant4_path = get_option('prefix')
geant4_pc = geant4_path / 'lib/pkgconfig/geant4.pc'
script_path = meson.project_source_root() / 'meson/install_geant4_root_pkgconfig.py'
if not fs.exists(geant4_pc)
    message('geant4.pc not found in ' + geant4_pc + ', running install script...')
    run_command(python_exe, script_path)
endif

# root: pkg-config created manually by./meson/install_geant4_root_pkgconfig.py
if get_option('use_root')
    root_deps = dependency('root',
                           version : '>=6.28/02',
                           include_type : 'preserve'
    )
endif
#root_deps = dependency('root', method: 'cmake')

# geant4: pkg-config created manually by ./meson/install_geant4_root_pkgconfig.py
geant4_deps = dependency('geant4',
                         version : '>=11.3.0',
                         include_type : 'preserve'
)
# this also works,but ONLY if the dynamic libraries are present
# geant4_deps = dependency('Geant4', method : 'cmake', static : true)


# sqlite
sqlite_dep = dependency('sqlite3',
                        include_type : 'preserve')

cmake = import('cmake')
opt_var = cmake.subproject_options()
opt_var.add_cmake_defines({ 'BUILD_SHARED_LIBS' : 'OFF', 'BUILD_STATIC_LIBS' : 'ON', 'CMAKE_POLICY_VERSION_MINIMUM' : '3.5' })
opt_var.append_compile_args('cpp', '-Wno-shadow')

assimp_proj = cmake.subproject('assimp', options : opt_var)
assimp_dep = assimp_proj.dependency('assimp')

yaml_cpp_proj = cmake.subproject('yaml-cpp', options : opt_var)
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
