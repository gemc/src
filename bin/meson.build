# gemc executable.
#
# This target lives in bin/ on purpose: Meson derives an executable's build-tree output path from
# the directory of the meson.build that declares it, so declaring it here puts the binary at
# build/bin/gemc. The alternatives both fail or warn: a 'bin/gemc' target name has a path separator
# (deprecated, becoming a hard error), and a plain 'gemc' name in the project root would collide
# with the build/gemc/ directory created by the subdir('gemc/...') calls.
#
# All variables referenced below (gemc_sources, qrc_gemc_sources, the *_dep dependencies and
# gemc_includes/all_libs) are defined in the parent meson.build before its subdir('bin') call;
# subdir() shares scope, so they are in scope here and the resulting `gemc` variable is visible to
# the parent again afterwards (used by the test() definitions).
gemc = executable(
    'gemc',
    gemc_sources + qrc_gemc_sources,
    install : true,
    install_dir : get_option('bindir'),
    dependencies : [yaml_cpp_dep, qt6_deps, clhep_deps, geant4_deps, ogl_deps, assimp_dep, expat_dep, zlib_dep],
    include_directories : gemc_includes,
    link_with : all_libs,
    # Export the executable's dynamic symbol table (GNU --export-dynamic / -rdynamic, and the
    # macOS equivalent). CLHEP and Geant4 are linked statically into gemc, so without this their
    # symbols stay private and dlopen'd .gplugin output/digitization modules fail to resolve them
    # at load (e.g. "undefined symbol: _Z9_G4cerr_pv", i.e. G4cerr's _G4cerr_p()). Exporting keeps
    # a single shared copy of G4/CLHEP global state (notably the CLHEP RNG) between gemc and its
    # plugins, rather than each plugin embedding its own.
    export_dynamic : true,
    override_options : ['b_pie=true']
)
