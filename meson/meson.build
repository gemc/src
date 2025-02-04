# qt: pkg-config. https://mesonbuild.com/Qt5-module.html
qt5 = import('qt5')
qt5_deps = dependency('qt5',
                      modules : ['Core', 'Gui', 'Widgets', 'OpenGL'],
                      include_type : 'preserve')

# clhep: pkg-config
clhep_deps = dependency('clhep',
                        version : '>=2.4.7.1',
                        include_type : 'preserve'
)

# geant4: pkg-config created manually by meson/create_and_install_geant4_pkgconfig.py
geant4_deps = dependency('geant4',
                         version : '>=11.3.0',
                         include_type : 'preserve'
)

# geant4: pkg-config created manually by meson/create_and_install_geant4_pkgconfig.py
root_deps = dependency('root',
                       version : '>=6.32.06',
                       include_type : 'preserve'
)

# sqlite
sqlite_dep = dependency('sqlite3',
                        include_type : 'preserve')

# Open Asset Import Library (assimp)
#assimp_dep = dependency('assimp',
#                        default_options : ['cpp_args=-Wno-shadow'],
#                        include_type : 'preserve') # looks like cmake_args is not working
cmake = import('cmake')
opt_var = cmake.subproject_options()
opt_var.add_cmake_defines({'BUILD_SHARED_LIBS': 'OFF'})

assimp_proj = cmake.subproject('assimp',
                               options : opt_var
)
assimp_dep = assimp_proj.dependency('assimp')


# yaml-cpp
yaml_cpp_dep = dependency('yaml_cpp',
                          default_options : ['cpp_args=-Wno-shadow'],
                          include_type : 'preserve') # looks like cmake_args is not working

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
